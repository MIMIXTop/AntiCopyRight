#pragma once

#include <QByteArray>
#include <QString>
#include <quazip/quazip.h>
#include <optional>
#include <vector>
#include <string>

namespace DocReader {
    std::optional<std::string> xmlReader(std::string &&xml);

    std::optional<std::string> zipReader(std::span<unsigned char> &&zip);
}
