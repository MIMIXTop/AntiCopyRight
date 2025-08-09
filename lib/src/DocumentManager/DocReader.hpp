#pragma once

#include <QByteArray>
#include <QString>
#include <quazip/quazip.h>
#include <optional>

namespace DocReader {
    std::optional<QString> readFile(QByteArray &document);
}
