#pragma once

#include <filesystem>

#include <client/cpp/synnax.h>

typedef struct {
    uint64_t timestamp; // in microseconds
    uint64_t data;
    uint64_t sensor_id;
} TelemetryPacket;

void telem_proxy(synnax::Synnax &client, int telem_socket, std::filesystem::path telem_config, std::atomic<bool> &shutdown);
