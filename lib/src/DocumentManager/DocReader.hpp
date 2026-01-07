#pragma once

#include <QByteArray>
#include <QString>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace DocReader {
std::optional<std::string> xmlReader(std::string &&xml);

std::optional<std::string> zipReader(std::span<unsigned char> &&zip);
} // namespace DocReader
