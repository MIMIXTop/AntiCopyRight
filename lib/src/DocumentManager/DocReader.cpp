
#include "DocReader.hpp"
#include "bit7z/bit7zlibrary.hpp"
#include "bit7z/bitformat.hpp"
#include "bit7z/bittypes.hpp"

#include <QString>
#include <QBuffer>
#include <QFile>
#include <map>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <QXmlStreamReader>
#include <string>

#include <pugixml.hpp>
#include <bit7z/bitmemextractor.hpp>

namespace  {
    struct DocWalker : public pugi::xml_tree_walker {
        std::string result;
        bool skipNextText = false;

       virtual bool for_each(pugi::xml_node& node) override {
            std::string name = node.name();
            if (name == "w:rFonts" && std::string(node.attribute("w:cs").value()) == "Courier New") {
                skipNextText = true;
            }
            if (name == "w:t") {
                if (skipNextText) {
                    skipNextText = false;
                    return true;
                }
                result += node.text().as_string();
            }

            return true;
        }
    };
}

std::optional<QString> DocReader::readFile(QByteArray &document) {
    QString result;

    QBuffer buffer(&document);
    buffer.open(QIODevice::ReadOnly);

    QuaZip quaZip(&buffer);
    if (!quaZip.open(QuaZip::mdUnzip)) {
        qInfo() << "Could not open quazip file" << document;
        return std::nullopt;
    }
    if (!quaZip.setCurrentFile("word/document.xml")) {
        qInfo() << "Not find document.xml file in " << document;
        quaZip.close();
        return std::nullopt;
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

std::optional<std::string> DocReader::readFile(std::vector<unsigned char> &&document) {
    bit7z::Bit7zLibrary lib{get7zLibraryPath()};
    std::vector<bit7z::byte_t> outData;
    bit7z::BitMemExtractor extractor{lib, bit7z::BitFormat::Zip};
    std::map<std::string, std::vector<bit7z::byte_t>> files;
    extractor.extract(document, files);

    if (files.contains("word/document.xml")) {
        const auto& data = files["word/document.xml"];
        return xmlReader(std::string(data.begin(), data.end()));
        //return std::string(reinterpret_cast<const char*>(data.data()), data.size());
    }

    return std::nullopt;
}  

std::optional<std::string> DocReader::xmlReader(std::string &&xml) {
    pugi::xml_document doc;
    doc.load_string(xml.c_str());
    DocWalker walker;
    doc.traverse(walker);
    return walker.result;
}

std::string DocReader::get7zLibraryPath() {
    if (const char* envPath = std::getenv("ANTYCOPY_7Z_PATH")) {
        return std::string(envPath);
    }

#ifdef SEVENZIP_LIB_PATH
    if (QFile::exists(SEVENZIP_LIB_PATH)) {
        return SEVENZIP_LIB_PATH;
    }
#endif

    const std::vector<std::string> paths = {
#ifdef WIN32
        "C:/Program Files/7-Zip/7z.dll",
        "C:/Program Files (x86)/7-Zip/7z.dll",
        "7z.dll"
#else
        "/usr/lib/7zip/7z.so",
        "/usr/lib/p7zip/7z.so",
        "/usr/local/lib/7zip/7z.so",
        "lib7z.so",
        "7z.so"
#endif
    };

    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

#ifdef WIN32
    return "7z.dll";
#else
    return "7z.so";
#endif
}

