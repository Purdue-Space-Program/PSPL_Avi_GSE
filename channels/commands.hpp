#pragma once

#include <filesystem>
#include <atomic>

#include "client/cpp/synnax.h"

void forward_commands(synnax::Synnax &client, int command_socket, std::filesystem::path command_config, std::atomic<bool> &shutdown);
