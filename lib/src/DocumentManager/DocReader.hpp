#ifndef DOCREADER_HPP
#define DOCREADER_HPP

#include <QString>
#include <QStringList>
#include <quazip/quazip.h>
#include  <optional>

#include "quazipfile.h"

class DocReader {

public:
    DocReader() = default;

    std::optional<QString> readFile(const QString &document);

    ~DocReader() = default;
};

#endif //DOCREADER_HPP
