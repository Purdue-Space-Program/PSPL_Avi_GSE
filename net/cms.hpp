#pragma once

#include <optional>
#include <filesystem>
#include <string>

int initialize_net_config();

//! Returns CommandNet socket, None if non-existant
std::optional<int> get_cmdnet_sock();

//! Returns telemetry socket, None if non-existant
std::optional<int> get_telem_sock();
