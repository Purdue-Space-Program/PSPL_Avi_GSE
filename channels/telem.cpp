#include "channels/telem.hpp"
#include "channels/synnax.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <fstream>
#include <optional>
#include <unordered_map>

static constexpr double ADC_V_SLOPE  =  424241635.4;
static constexpr double ADC_V_OFFSET = -0.01390133824020600000;

void telem_proxy(synnax::Synnax &client, int telem_socket, std::filesystem::path telem_config, std::atomic<bool> &shutdown) {
    std::cout << "Starting up Telemetry Proxy..." << '\n';
    std::ifstream avi_file(telem_config);

    if (!avi_file.is_open()) {
        std::cout << "Error! Not able to open telemetry config file!" << '\n';
        shutdown.store(true);
        return;
    }

    nlohmann::json file_json;
    avi_file >> file_json;

    synnax::WriterConfig writer_config;
    writer_config.enable_auto_commit = true;
    writer_config.mode = synnax::WriterMode::PersistStream;

    // add all channels to writer
    for (auto& [key, value] : file_json.items()) {
        auto ch = client.channels.retrieve(key);
        auto time_ch = client.channels.retrieve(key + "_time");
        if (ch.second.ok() && time_ch.second.ok()) {
            writer_config.channels.push_back(ch.first.key);
            writer_config.channels.push_back(time_ch.first.key);
        } else {
            std::cout << "Error retrieving channel: " << key << '\n';
            shutdown.store(true);
            return;
        }
    }

    // make channels indexable by ID
    struct ChannelConfig {
        ChannelConfig() = default;

        std::string name;
        std::optional<std::string> unit;
        std::string type;
        std::optional<double> slope;
        std::optional<double> offset;
    };

    std::unordered_map<std::uint8_t, ChannelConfig> telem_configs;
    for (auto& [key, value] : file_json.items()) {
        auto config = ChannelConfig();
        config.slope  = value.contains("slope") 
            ? std::make_optional(value.at("slope").get<double>()) 
            : std::nullopt;
        config.offset = value.contains("offset") 
            ? std::make_optional(value.at("offset").get<double>())
            : std::nullopt;

        if (value.contains("zero_offset") && value.contains("offset")) {
            config.offset.value() += value.at("zero_offset").get<double>();
        }
        config.unit = value.contains("unit") 
            ? std::make_optional(value.at("unit").get<std::string>())
            : std::nullopt;

        config.name = key;
        config.type = value.at("channel_type").get<std::string>();

        telem_configs.insert_or_assign(value.at("id").get<std::uint8_t>(), config);
    }

    writer_config.start = telem::TimeStamp::now();
    std::pair<synnax::Writer, xerrors::Error> writer = client.telem.open_writer(writer_config);

    if (writer.second.ok()) {
        while (true) {
            if (shutdown.load()) {
                std::cout << "Received shutdown command!!" << '\n';
                return;
            }

            std::array<gse::TelemetryPacket, 25> packets;
            // need to make non-blocking :(
            auto ret = read(telem_socket, packets.data(), sizeof(gse::TelemetryPacket) * 25);
            if (ret < 1 && ret != (sizeof(gse::TelemetryPacket) * 25)) {
                std::cout << "TCP read error!" << '\n';
                shutdown.store(true);
                return;
            }
            
            std::size_t count = ret / sizeof(gse::TelemetryPacket);
            for (std::size_t i = 0; i < count; i += 1) {
                gse::TelemetryPacket packet = std::move(packets[i]);

                if (telem_configs.count(packet.sensor_id) == 0) {
                    std::cout << "Could not find channel of ID: " << packet.sensor_id << '\n';
                    continue;
                }

                auto channel_config = telem_configs[packet.sensor_id];

                double calibrated_data = packet.data;
                if (channel_config.slope.has_value()) {
                    if (channel_config.unit == "psi") {
                        double data_float = static_cast<double>(static_cast<std::int32_t>(packet.data));
                        double v_calibrated_data = data_float / ADC_V_SLOPE + ADC_V_OFFSET;
                        calibrated_data = v_calibrated_data * channel_config.slope.value() + channel_config.offset.value();
                    } else {
                        calibrated_data = packet.data * channel_config.slope.value() + channel_config.offset.value();
                    }
                }

                auto data_type = type_from_text(channel_config.type).value();
                telem::SampleValue value = calibrated_data;

                synnax::Frame frame(2);

                auto sy_ch = client.channels.retrieve(channel_config.name);
                if (!sy_ch.second.ok()) {
                    std::cout << "Failed to find channel of name: " << channel_config.name << '\n';
                    shutdown.store(true);
                    return;
                }

                frame.emplace(sy_ch.first.key, telem::Series(value));
                frame.emplace(sy_ch.first.index, telem::Series(telem::TimeStamp(static_cast<std::int64_t>(packet.timestamp))));

                auto write_ret = writer.first.write(frame);
                if (!write_ret.ok()) {
                    std::cout << "Failed write: " << write_ret.message() << '\n';
                    shutdown.store(true);
                    return;
                }
            }
        }
    } else {
        std::cout << "Could not open telemetry writer!" << '\n';
        shutdown.store(true);
        return;
    }
}
