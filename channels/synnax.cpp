#include <client/cpp/synnax.h>
#include <nlohmann/json.hpp>

#include <optional>

#include "channels/synnax.hpp"

std::shared_ptr<synnax::Synnax> synnax_client;

bool connect_to_synnax() {
    if (synnax_client && synnax_client->auth->authenticate().ok()) {
        return true;
    }

    synnax::Config config;

    std::filesystem::path synnax_config = "pspl_ops_config/synnax.json";

    std::ifstream file(std::filesystem::absolute(synnax_config));
    if (!file.is_open()) {
        std::cout << "Unable to open Synnax config file at: \"" << 
            synnax_config <<
            "\"\n";
        return false;
    }
    json file_json;
    file >> file_json;

    config.host = file_json.at("hostname").get<std::string>();
    config.port = file_json.at("port").get<std::uint16_t>();
    config.username = file_json.at("username").get<std::string>();
    config.password = file_json.at("password").get<std::string>();

    synnax_client = std::make_shared<synnax::Synnax>(config);

    return synnax_client->auth->authenticate().ok();
}

std::optional<telem::DataType> type_from_text(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(),
    [](unsigned char c){ return std::tolower(c); });

    if      (type == "f64") return telem::FLOAT64_T;
    else if (type == "f32") return telem::FLOAT32_T;
    else if (type == "u64") return telem::UINT64_T;
    else if (type == "u32") return telem::UINT32_T;
    else if (type == "u16") return telem::UINT16_T;
    else if (type == "u8")  return telem::UINT8_T;
    else if (type == "i64") return telem::INT64_T;
    else if (type == "i32") return telem::INT32_T;
    else if (type == "i16") return telem::INT16_T;
    else if (type == "i8")  return telem::INT8_T;
    else return {};
}

std::optional<std::uint8_t> size_from_type(telem::DataType type) {
    if (type == telem::FLOAT64_T) return 8;
    else if (type == telem::FLOAT32_T) return 4;
    else if (type == telem::UINT64_T) return 8;
    else if (type == telem::UINT32_T) return 4;
    else if (type == telem::UINT16_T) return 2;
    else if (type == telem::UINT8_T) return 1;
    else if (type == telem::INT64_T) return 8;
    else if (type == telem::INT32_T) return 4;
    else if (type == telem::INT16_T) return 2;
    else if (type == telem::INT8_T) return 1;
    else return {};
}
