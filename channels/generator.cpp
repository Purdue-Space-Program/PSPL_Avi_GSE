#include "client/cpp/synnax.h"
#include <nlohmann/json.hpp>

#include "channels/generator.hpp"
#include "channels/synnax.hpp"

xerrors::Error create_channel(synnax::Synnax &client, std::string name, telem::DataType type) {
    synnax::Channel time_ch;
    time_ch.name = name + "_time";
    time_ch.data_type = telem::TIMESTAMP_T;
    time_ch.is_index = true;

    auto ret = client.channels.create(time_ch);
    if (!ret.ok()) return ret;

    synnax::Channel ch;
    ch.name = name;
    ch.data_type = type;
    ch.is_index = false;
    ch.index = time_ch.key;

    ret = client.channels.create(ch);

    return {};
}

xerrors::Error create_virtual_channel(synnax::Synnax &client, std::string name, telem::DataType type) {
    synnax::Channel ch;
    ch.name = name;
    ch.data_type = type;
    ch.is_virtual = true;

    return client.channels.create(ch);
}

xerrors::Error add_telem_channels(synnax::Synnax &client, std::string filename) {
    std::ifstream avi_file(filename);

    if (!avi_file.is_open()) {
        // handle error
        std::cout << "Error! Not able to open telemetry config file!" << '\n';
        return {};
    }

    json file_json;
    avi_file >> file_json;

    std::cout << "Adding telemetry channels..." << '\n'; 
    for (auto& [key, value] : file_json.items()) {
        auto ret = create_channel(client, key, type_from_text(value.at("channel_type").get<std::string>()).value());
        if (ret.ok()) {
            std::cout << "\tAdded \033[1m" << key << "\033[0m\n";
        } else {
            std::cout << "\tReconfigured \033[1m" << key << "\033[0m\n";
        }
    }

    return {};
}

xerrors::Error add_command_channels(synnax::Synnax &client, std::string filename) {
    std::ifstream avi_file(filename);

    if (!avi_file.is_open()) {
        // handle error
        std::cout << "Error! Not able to open command config file!" << '\n';
        return {};
    }

    json file_json;
    avi_file >> file_json;

    std::cout << "Adding command channels..." << '\n'; 
    for (auto& [key, value] : file_json.items()) {
        auto ret = create_virtual_channel(client, key + "_cmd", type_from_text(value.at("channel_type").get<std::string>()).value());
        if (ret.ok()) {
            std::cout << "\tAdded \033[1m" << key << "\033[0m\n";
        } else {
            std::cout << "\tReconfigured \033[1m" << key << "\033[0m\n";
        }
    }

    return {};
}
