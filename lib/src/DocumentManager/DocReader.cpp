
#include "DocReader.hpp"

#include <QString>
#include <quazip/quazip.h>
#include <QXmlStreamReader>


std::optional<QString> DocReader::readFile(const QString &document) {
    QString result;

    QuaZip quaZip(document);
    if (!quaZip.open(QuaZip::mdUnzip)) {
        qInfo() << "Could not open quazip file" << document;
        return std::nullopt;
    }
    if (!quaZip.setCurrentFile("word/document.xml")) {
        qInfo() << "Not find document.xml file in " << document;
        quaZip.close();
        return  std::nullopt;
    }

    QuaZipFile quaZipFile(&quaZip);

    if (!quaZipFile.open(QIODevice::ReadOnly)) {
        qInfo() << "Could not open document.xml in file" << document;
        quaZipFile.close();
        return std::nullopt;
    }

    QXmlStreamReader reader(&quaZipFile);
    bool needReadText = true;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.name() == "rFonts" && reader.attributes().value("w:cs").toString() == "Courier New") {
            needReadText = false;
        }

        if (reader.name() == "t") {
            reader.readNext();
            if (!needReadText) {
                needReadText = true;
                continue;
            }

            if (reader.isCharacters() && !reader.isWhitespace()) {
                result.append(reader.text().toString());
            }
        }
    }

    if (reader.hasError()) {
        qInfo() << "Failed XML:" << reader.errorString();
    }

    reader.clear();
    quaZipFile.close();

    return result;
}

