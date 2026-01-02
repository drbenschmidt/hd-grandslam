cmake_minimum_required(VERSION 3.25)
project(hd-grandslam LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

find_package(cpr REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Crow REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(FFmpeg REQUIRED COMPONENTS avformat avcodec avutil swscale)

add_executable(hdhr_streamer
    src/main.cpp
    src/HDHRClient.cpp
    src/Streamer.cpp
)

target_include_directories(hdhr_streamer PRIVATE src)

target_link_libraries(hdhr_streamer PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
    Crow::Crow
    spdlog::spdlog
    fmt::fmt
    ffmpeg::avformat
    ffmpeg::avcodec
    ffmpeg::avutil
    ffmpeg::swscale
)

# platform-specific: on Windows link ws2_32
if(WIN32)
    target_link_libraries(hdhr_streamer PRIVATE ws2_32)
endif()
