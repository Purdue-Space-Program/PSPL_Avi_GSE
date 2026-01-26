#include "channels/telem.hpp"
#include "channels/synnax.hpp"

#include <nlohmann/json.hpp>

#include <sys/socket.h>

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
        DataType type;
        std::optional<double> slope;
        std::optional<double> offset;
        synnax::Channel sy_channel;
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

        // assuming good
        config.type = type_from_str(value.at("channel_type").get<std::string>()).value();

        // assuming it exists which probably isnt good
        auto ret = client.channels.retrieve(key);
        if (!ret.second.ok()) {
            std::cout << "Could not find channel " << key << '!' << '\n';
            shutdown.store(true);
            return;
        }
        config.sy_channel = client.channels.retrieve(key).first;

        telem_configs.insert_or_assign(value.at("id").get<std::uint8_t>(), config);
    }

    writer_config.start = telem::TimeStamp::now();
    std::pair<synnax::Writer, xerrors::Error> writer = client.telem.open_writer(writer_config);

    if (writer.second.ok()) {

        int ret = 0;
        while (true) {
            if (shutdown.load()) {
                std::cout << "Received shutdown command!!" << '\n';
                return;
            }

            std::array<gse::TelemetryPacket, 25> packets;
            // need to make non-blocking :(

            gse::TelemetryPacket _dummy;
            if ((ret % sizeof(gse::TelemetryPacket)) != 0) {
                auto leftover_size = sizeof(gse::TelemetryPacket) - ret % sizeof(gse::TelemetryPacket);
                recv(telem_socket, &_dummy, leftover_size, 0);
                std::cout << "Cancelling " << leftover_size << " bytes!" << '\n';
            }

            ret = recv(telem_socket, packets.data(), sizeof(gse::TelemetryPacket) * 25, 0);
            if (ret < 1) {
                std::cout << "TCP read error!" << '\n';
                shutdown.store(true);
                return;
            }
            
            std::size_t count = ret / sizeof(gse::TelemetryPacket);
            for (std::size_t i = 0; i < count; i += 1) {
                gse::TelemetryPacket packet = std::move(packets[i]);

                if (telem_configs.count(packet.sensor_id) == 0) {
                    std::cout << "Could not find channel of ID: " << packet.sensor_id << '\n';
                    shutdown.store(true);
                    return;
                }

                auto channel_config = telem_configs[packet.sensor_id];

                telem::SampleValue value;

                switch (channel_config.type) {
                    case DataType::FLOAT64:
                        value = static_cast<double>(packet.data);
                        break;
                    case DataType::FLOAT32:
                        value = static_cast<float>(packet.data);
                        break;
                    case DataType::UINT8:
                        value = static_cast<std::uint8_t>(packet.data);
                        break;
                    case DataType::UINT16:
                        value = static_cast<std::uint16_t>(packet.data);
                        break;
                    case DataType::UINT32:
                        value = static_cast<std::uint32_t>(packet.data);
                        break;
                    case DataType::UINT64:
                        value = static_cast<std::uint64_t>(packet.data);
                        break;
                    case DataType::INT8:
                        value = static_cast<std::int8_t>(packet.data);
                        break;
                    case DataType::INT16:
                        value = static_cast<std::int16_t>(packet.data);
                        break;
                    case DataType::INT32:
                        value = static_cast<std::int32_t>(packet.data);
                        break;
                    case DataType::INT64:
                        value = static_cast<std::int64_t>(packet.data);
                        break;
                }

                if (channel_config.slope.has_value()) {
                    if (channel_config.unit == "psi") {
                        double data_float = static_cast<double>(static_cast<std::int32_t>(packet.data));
                        double v_calibrated_data = data_float / ADC_V_SLOPE + ADC_V_OFFSET;
                        value = v_calibrated_data * channel_config.slope.value() + channel_config.offset.value();
                    } else {
                        value = packet.data * channel_config.slope.value() + channel_config.offset.value();
                    }
                }

                synnax::Frame frame(2);

                frame.emplace(channel_config.sy_channel.key, telem::Series(value));
                frame.emplace(channel_config.sy_channel.index, telem::Series(telem::TimeStamp(static_cast<std::int64_t>(packet.timestamp))));

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

std::optional<DataType> type_from_str(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
    [](unsigned char c){ return std::tolower(c); });
    
    if (str == "f64") return DataType::FLOAT64;
    else if (str == "f32") return DataType::FLOAT32;
    else if (str == "u8") return DataType::UINT8;
    else if (str == "u16") return DataType::UINT16;
    else if (str == "u32") return DataType::UINT32;
    else if (str == "u64") return DataType::UINT64;
    else if (str == "i8") return DataType::INT8;
    else if (str == "i16") return DataType::INT16;
    else if (str == "i32") return DataType::INT32;
    else if (str == "i64") return DataType::INT64;
    else return std::nullopt;
}
