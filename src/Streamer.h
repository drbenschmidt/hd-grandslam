#pragma once


#include <string>
#include <memory>


class Streamer {
public:
// Build and start ffmpeg subprocess to transcode hdhr_url and stream to stdout
// Returns a FILE* to read ffmpeg stdout (caller must close)
static FILE* start_ffmpeg_pipe(const std::string& hdhr_url);
};