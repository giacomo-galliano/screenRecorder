#include "../include/GetAudioDevices.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#ifdef __linux__
using namespace std;
#endif

std::vector<std::string> getAudioDevices() {
    try {
        std::vector<std::string> devices;
#ifdef __linux__
        filesystem::directory_iterator alsaDir{"/proc/asound"};
        regex findCard{".*card(0|[1-9]?[0-9]*)"};
        regex findPcm{".*pcm.*"};
        std::string value, field, card, device, streamType;

        for (auto i : alsaDir) {
            if (regex_match(i.path().c_str(), findCard)) {
                filesystem::directory_iterator cardDir{i.path()};
                for (auto const& j : cardDir) {
                    if (regex_match(j.path().c_str(), findPcm)) {
                        ifstream info{(std::string)j.path() + "/info"};
                        if (info.is_open()) {
                            while (!info.eof()) {
                                info >> field;
                                info >> value;
                                if (field == "card:") {
                                    card = value;
                                } else if (field == "device:") {
                                    device = value;
                                } else if (field == "stream:") {
                                    streamType = value;
                                }
                            }
                            if (streamType == "CAPTURE") {
                                devices.emplace_back("hw:" + card + "," + device);
                            }
                        } else {
                            throw runtime_error{"Error opening device file"};
                        }
                    }
                }
            }
        }
#endif
        return devices;
    } catch (const std::exception& e) {
        return std::vector<std::string>{};
    }
}