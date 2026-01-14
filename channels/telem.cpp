#include "channels/telem.hpp"
#include <optional>

void telem_proxy(synnax::Synnax &client, int telem_socket, std::filesystem::path telem_config, std::atomic<bool> &shutdown) {
    std::cout << "Starting up Telemetry Proxy..." << '\n';
    std::ifstream avi_file(telem_config);

    if (!avi_file.is_open()) {
        std::cout << "Error! Not able to open telemetry config file!" << '\n';
        return;
    }

    json file_json;
    avi_file >> file_json;

    synnax::WriterConfig writer_config;
    writer_config.enable_auto_commit = true;
    writer_config.start = telem::TimeStamp::now();
    writer_config.mode = synnax::WriterMode::PersistStream;

    for (auto& [key, value] : file_json.items()) {
        auto ch = client.channels.retrieve(key);
        if (ch.second.ok()) {
            writer_config.channels.push_back(ch.first.key);
        } else {
            std::cout << "Error retrieving channel: " << key << '\n';
            return;
        }
    }

    auto writer = client.telem.open_writer(writer_config);
    std::cout << "Writer: " << writer.second.ok() << '\n';

    while (true) {
        TelemetryPacket packet;
        auto ret = read(telem_socket, &packet, sizeof(TelemetryPacket));
        if (ret == -1) {
            std::cout << "Read error!" << '\n';
            return;
        }

        std::optional<nlohmann::basic_json<>> channel_config = {};
        for (auto& [key, value] : file_json.items()) {
            if (value["id"] == packet.sensor_id) {
                channel_config = value;
                break;
            }
        }

        // id not found
        if (!channel_config) {
            std::cout << "Could not find channel of ID: " << packet.sensor_id << '\n';
            return;
        }

        if (channel_config.value()["unit"] == "psi") {
            // read data as a i32 or smth
            // apply voltage calibration to data
            // apply calibration
        } else {
            // apply calibration normally
        }

        // write data to Synnax
    }
}
