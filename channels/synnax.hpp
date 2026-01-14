#pragma once

#include <client/cpp/synnax.h>

#include <optional>

extern std::shared_ptr<synnax::Synnax> synnax_client;

//! returns true if successfully connected
bool connect_to_synnax();
std::optional<telem::DataType> type_from_text(std::string type);
std::optional<std::uint8_t> size_from_type(telem::DataType type);
