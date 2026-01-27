#pragma once

#include <optional>
#include <span>
#include <string>

namespace DocReader {
std::optional<std::string> xmlReader(std::string&& xml);

std::optional<std::string> zipReader(std::span<unsigned char>&& zip);
}   // namespace DocReader
