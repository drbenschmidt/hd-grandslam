#include "Streamer.h"
#include <vector>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
}

// Context passed to AVIO writer
struct AVIOUserContext {
    crow::response* res;
};

// libav write callback → writes bytes directly to the HTTP response
static int avio_write_packet(void* opaque, uint8_t* buf, int buf_size) {
    AVIOUserContext* ctx = static_cast<AVIOUserContext*>(opaque);
    if (!ctx || !ctx->res) return AVERROR(EIO);

    try {
        ctx->res->write(std::string(reinterpret_cast<const char*>(buf),
                        static_cast<size_t>(buf_size)));
        // ctx->res->flush();
    } catch (...) {
        return AVERROR(EIO);
    }

    return buf_size;
}

// No seeking in HTTP live streams
static int64_t avio_seek_packet(void*, int64_t, int) {
    return -1;
}

bool Streamer::stream_via_libav(const std::string& hdhr_url, crow::response& res) {
    avformat_network_init();

    AVFormatContext* ifmt_ctx = nullptr;
    int ret = avformat_open_input(&ifmt_ctx, hdhr_url.c_str(), nullptr, nullptr);
    if (ret < 0) {
        res.code = 500;
        res.write("Failed to open HDHR stream\n");
        res.end();
        return false;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, nullptr)) < 0) {
        res.code = 500;
        res.write("Failed to read HDHR stream info\n");
        res.end();
        avformat_close_input(&ifmt_ctx);
        return false;
    }

    // Create output MP4 (fragmented)
    AVFormatContext* ofmt_ctx = nullptr;
    ret = avformat_alloc_output_context2(&ofmt_ctx, nullptr, "mp4", nullptr);
    if (!ofmt_ctx) {
        res.code = 500;
        res.write("Failed to create MP4 output context\n");
        res.end();
        avformat_close_input(&ifmt_ctx);
        return false;
    }

    // Build stream map
    std::vector<int> stream_mapping(ifmt_ctx->nb_streams, -1);
    for (unsigned i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream* in_stream = ifmt_ctx->streams[i];
        auto* in_par = in_stream->codecpar;

        if (in_par->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_par->codec_type != AVMEDIA_TYPE_VIDEO) {
            continue;
        }

        AVStream* out_stream = avformat_new_stream(ofmt_ctx, nullptr);
        if (!out_stream) {
            res.code = 500;
            res.write("Failed creating MP4 stream\n");
            res.end();
            avformat_close_input(&ifmt_ctx);
            avformat_free_context(ofmt_ctx);
            return false;
        }

        avcodec_parameters_copy(out_stream->codecpar, in_par);
        out_stream->codecpar->codec_tag = 0;

        stream_mapping[i] = out_stream->index;
    }

    // Create custom AVIO writer → HTTP response
    const int io_buf_size = 32 * 1024;
    unsigned char* io_buf = (unsigned char*)av_malloc(io_buf_size);

    AVIOUserContext user_ctx{ &res };

    AVIOContext* avio_ctx = avio_alloc_context(
        io_buf, io_buf_size, 1, &user_ctx,
        nullptr, // read
        avio_write_packet,
        avio_seek_packet
    );

    ofmt_ctx->pb = avio_ctx;

    // Required flags for fragmented MP4
    AVDictionary* mux_opts = nullptr;
    av_dict_set(&mux_opts, "movflags", "frag_keyframe+empty_moov", 0);

    ret = avformat_write_header(ofmt_ctx, &mux_opts);
    av_dict_free(&mux_opts);
    if (ret < 0) {
        res.code = 500;
        res.write("Failed writing MP4 header\n");
        res.end();
        avformat_close_input(&ifmt_ctx);
        avformat_free_context(ofmt_ctx);
        return false;
    }

    // Begin streaming output
    res.code = 200;
    res.set_header("Content-Type", "video/mp4");
    res.set_header("Transfer-Encoding", "chunked");

    AVPacket pkt;
    av_init_packet(&pkt);

    while (true) {
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) break;

        int in_index = pkt.stream_index;
        if (in_index >= (int)stream_mapping.size() || stream_mapping[in_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }

        pkt.stream_index = stream_mapping[in_index];

        AVStream* in_stream = ifmt_ctx->streams[in_index];
        AVStream* out_stream = ofmt_ctx->streams[pkt.stream_index];

        // Rescale PTS/DTS
        pkt.pts = av_rescale_q(pkt.pts, in_stream->time_base, out_stream->time_base);
        pkt.dts = av_rescale_q(pkt.dts, in_stream->time_base, out_stream->time_base);
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        av_packet_unref(&pkt);

        if (ret < 0) break; // client closed or error
    }

    av_write_trailer(ofmt_ctx);

    // Cleanup
    av_freep(&avio_ctx->buffer);
    avio_context_free(&avio_ctx);

    avformat_close_input(&ifmt_ctx);
    avformat_free_context(ofmt_ctx);

    res.end();
    return true;
}
