#pragma once

#include <filesystem>
#include <optional>

#include "client/cpp/synnax.h"

namespace gse {
    struct TelemetryPacket {
        TelemetryPacket() = default;

        uint64_t timestamp; // in microseconds
        uint64_t data;
        uint64_t sensor_id;
    };
}

void telem_proxy(synnax::Synnax &client, int telem_socket, std::filesystem::path telem_config, std::atomic<bool> &shutdown);

enum DataType : std::uint8_t {
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    INT8,
    INT16,
    INT32,
    INT64,
    FLOAT32,
    FLOAT64,
};

std::optional<DataType> type_from_str(std::string str);
