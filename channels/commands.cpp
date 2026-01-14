#include <sys/socket.h>

#include <nlohmann/json.hpp>
#include <client/cpp/synnax.h>

#include "channels/generator.hpp"
#include "channels/synnax.hpp"
#include "channels/commands.hpp"

static constexpr double ADC_V_SLOPE  =  424241635.4;
static constexpr double ADC_V_OFFSET = -0.01390133824020600000;

void forward_commands(synnax::Synnax &client, int command_socket, std::filesystem::path command_config, std::atomic<bool> &shutdown) {
    std::cout << "Starting up Command Proxy..." << '\n';
    std::ifstream avi_file(command_config);

    if (!avi_file.is_open()) {
        std::cout << "Error! Not able to open command config file!" << '\n';
        return;
    }

    json file_json;
    avi_file >> file_json;

    synnax::StreamerConfig streamer_config;
    for (auto& [key, value] : file_json.items()) {
        auto ch = client.channels.retrieve(key + "_cmd");
        if (ch.second.ok()) {
            streamer_config.channels.push_back(ch.first.key);
        } else {
            std::cout << "Error retrieving channel: " << key + "_cmd" << '\n';
            return;
        }
    }

    auto streamer_ret = client.telem.open_streamer(streamer_config);
    if (streamer_ret.second.ok()) {
        auto streamer = std::move(streamer_ret.first);
        while (true) {
            auto read = streamer.read();

            if (read.second.ok()) {
                auto frame = std::move(read.first);
                for (auto c : frame) {
                    auto channel = client.channels.retrieve(c.first);
                    
                    std::string stripped_name = channel.first.name.substr(0, channel.first.name.length() - 4);

                    auto channel_config = file_json.at(stripped_name);

                    int channel_num_args = channel_config.at("num_args").get<int>();

                    // assuming data_type can be handled by `size_from_type` function
                    int buffer_size = 1 + size_from_type(channel.first.data_type).value() * channel_num_args;

                    std::uint8_t channel_id = channel_config.at("id").get<std::uint8_t>();

                    std::vector<std::uint8_t> buffer(buffer_size);

                    // apply reverse calibration
                    if (channel_config.contains("slope")) {
                        double channel_slope = channel_config.at("slope");
                        double channel_offset = channel_config.at("offset").get<double>();
                        double channel_zero_offset = channel_config.at("zero_offset").get<double>();

                        auto v = c.second.at(0);

                        double raw_val = 0.0;
                        bool found = true;

                        if      (auto* val = std::get_if<double>(&v))         raw_val = (double)*val;
                        else if (auto* val = std::get_if<float>(&v))          raw_val = (double)*val;
                        else if (auto* val = std::get_if<long>(&v))           raw_val = (double)*val;
                        else if (auto* val = std::get_if<unsigned long>(&v))  raw_val = (double)*val;
                        else if (auto* val = std::get_if<int>(&v))            raw_val = (double)*val;
                        else if (auto* val = std::get_if<unsigned int>(&v))   raw_val = (double)*val;
                        else if (auto* val = std::get_if<short>(&v))          raw_val = (double)*val;
                        else if (auto* val = std::get_if<unsigned short>(&v)) raw_val = (double)*val;
                        else if (auto* val = std::get_if<signed char>(&v))    raw_val = (double)*val;
                        else if (auto* val = std::get_if<unsigned char>(&v))  raw_val = (double)*val;
                        else {
                            std::cout << "Unknown or empty variant state" << '\n';
                            found = false;
                        }

                        if (found) {
                            double calibrated = ((((raw_val - channel_offset - channel_zero_offset) / channel_slope)) - ADC_V_OFFSET) * ADC_V_SLOPE;

                            std::uint64_t converted = static_cast<std::uint64_t>(calibrated);

                            std::memcpy(buffer.data() + 1, &converted, sizeof(converted));
                        }
                    }
                    buffer[0] = channel_id;

                    if (send(command_socket, buffer.data(), buffer.size(), 0) <= 1) {
                        std::cout << "Error sending on Command socket!" << '\n';
                        shutdown.store(true);
                        return;
                    }

                    if (recv(command_socket, buffer.data(), 1, 0) <= 1) {
                        std::cout << "Error sending on Telemetry socket!" << '\n';
                        shutdown.store(true);
                        return;
                    }
                }
            } else {
                std::cout << "Bad frame detected. Continuing." << '\n';
            }
        }
    } else {
        std::cout << "Problem opening streamer..." << '\n';
    }
}
