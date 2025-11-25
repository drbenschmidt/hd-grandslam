#include <crow.h>
#include <thread>
#include <array>

int main(int argc, char **argv)
{
    crow::App<> app;
    HDHRClient client;

    CROW_ROUTE(app, "/api/discover/<string>")([&client](const std::string &ip) {
        auto dev = client.discover(ip);
        if (!dev) return crow::response(404, "Device not found");
        nlohmann::json j;
        j["DeviceID"] = dev->DeviceID;
        j["DeviceAuth"] = dev->DeviceAuth;
        j["BaseURL"] = dev->BaseURL;
        return crow::response{j.dump()};
    });

    CROW_ROUTE(app, "/api/lineup/<string>")([&client](const std::string &ip) {
        auto lineup = client.get_lineup(ip);
        nlohmann::json j = nlohmann::json::array();
        for (auto &c : lineup) {
            j.push_back({{"GuideName", c.GuideName}, {"GuideNumber", c.GuideNumber}, {"URL", c.URL}, {"HD", c.HD}});
        }
        return crow::response{j.dump()};
    });

    CROW_ROUTE(app, "/api/epg/<string>")([&client](const std::string &device_auth) {
        try {
            auto j = client.get_epg(device_auth);
            return crow::response{j.dump()};
        } catch(const std::exception &e) {
            return crow::response(500, std::string("EPG error: ") + e.what());
        }
    });

    // Stream endpoint: /stream/<device_id>/<channel>
    CROW_ROUTE(app, "/stream/<string>/<string>")
    ([&hdhr](const std::string& device_id, const std::string& channel) {
        crow::response res;
        res.code = 200;

        // streaming headers
        res.set_header("Content-Type", "video/mp4");
        res.set_header("Connection", "close");
        res.manual_write(); // allow incremental writes

        // Build HDHR stream URL
        std::string url = hdhr.stream_url(device_id, channel);

        Streamer streamer;
        streamer.stream_via_libav(url, res);

        return res;
    });

    // Serve minimal static UI
    CROW_ROUTE(app, "/")([](){
        return crow::response(200, "<html><body><h1>HDHR Streamer</h1><p>Use /api endpoints</p></body></html>");
    });

    spdlog::info("Starting HDHR Streamer on :8080");
    app.port(8080).multithreaded().run();
}
