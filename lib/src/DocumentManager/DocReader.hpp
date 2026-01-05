#pragma once

#include <QByteArray>
#include <QString>
#include <quazip/quazip.h>
#include <optional>
#include <vector>
#include <string>

namespace DocReader {
    std::optional<QString> readFile(QByteArray &document);

    std::optional<std::string> readFile(std::vector<unsigned char> &&document);

    std::optional<std::string> xmlReader(std::string &&xml);

    std::string get7zLibraryPath();
}
