#pragma once
#include <crow.h>
#include <string>
#include <memory>

class Streamer {
public:
static bool stream_via_libav(const std::string& hdhr_url, crow::response& res);
};