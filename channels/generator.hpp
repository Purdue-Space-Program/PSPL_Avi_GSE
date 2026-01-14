#pragma once

#include <client/cpp/synnax.h>
#include <optional>

xerrors::Error create_channel(synnax::Synnax &client, std::string name, telem::DataType type);
xerrors::Error create_virtual_channel(synnax::Synnax &client, std::string name, telem::DataType type);

xerrors::Error add_telem_channels(synnax::Synnax &client, std::string filename);
xerrors::Error add_command_channels(synnax::Synnax &client, std::string filename);
