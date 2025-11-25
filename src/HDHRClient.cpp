#include "HDHRClient.h"
#include <cpr/cpr.h>
#include <stdexcept>

using json = nlohmann::json;

std::optional<DeviceInfo> HDHRClient::discover(const std::string &device_ip)
{
    std::string url = "http://" + device_ip + "/discover.json";
    auto r = cpr::Get(cpr::Url{url});
    if (r.status_code != 200)
    {
        return std::nullopt;
    }
    auto j = json::parse(r.text);
    DeviceInfo d;
    d.DeviceID = j.value("DeviceID", "");
    d.DeviceAuth = j.value("DeviceAuth", "");
    d.BaseURL = j.value("BaseURL", "http://" + device_ip);
    return d;
}

std::vector<ChannelInfo> HDHRClient::get_lineup(const std::string &device_ip)
{
    std::string url = "http://" + device_ip + "/lineup.json";
    auto r = cpr::Get(cpr::Url{url});
    if (r.status_code != 200)
    {
        return {};
    }
    auto j = json::parse(r.text);
    std::vector<ChannelInfo> out;
    for (auto &item : j)
    {
        ChannelInfo c;
        c.GuideName = item.value("GuideName", "");
        c.GuideNumber = item.value("GuideNumber", "");
        c.URL = item.value("URL", "");
        c.HD = item.value("HD", false);
        out.push_back(std::move(c));
    }
    return out;
}

nlohmann::json HDHRClient::get_epg(const std::string &device_auth)
{
    std::string url = "https://api.hdhomerun.com/api/guide.php?DeviceAuth=" + device_auth;
    auto r = cpr::Get(cpr::Url{url});
    if (r.status_code != 200)
    {
        throw std::runtime_error("Failed to fetch EPG");
    }
    return json::parse(r.text);
}