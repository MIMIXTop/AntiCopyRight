#pragma once

#include <QString>
#include <QStringList>
#include <quazip/quazip.h>
#include  <optional>

#include "quazipfile.h"

class DocReader {

public:
    DocReader() = default;

    static std::optional<QString> readFile(const QString &document);

    ~DocReader() = default;
};
