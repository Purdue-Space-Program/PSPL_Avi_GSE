#pragma once

#include <filesystem>

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
