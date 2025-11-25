#pragma once


#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>


struct ChannelInfo {
std::string GuideName;
std::string GuideNumber;
std::string URL;
bool HD = false;
};


struct DeviceInfo {
std::string DeviceID;
std::string DeviceAuth;
std::string BaseURL;
};


class HDHRClient {
public:
HDHRClient() = default;


// Retrieve discover.json from the device
std::optional<DeviceInfo> discover(const std::string& device_ip);


// Get channel lineup from device (lineup.json)
std::vector<ChannelInfo> get_lineup(const std::string& device_ip);


// Fetch EPG from SiliconDust using DeviceAuth
nlohmann::json get_epg(const std::string& device_auth);
};
