#include "DocumentManager/DocReader.hpp"

#include <gtest/gtest.h>
#include <QFile>
#include <QBuffer>
#include <QByteArray>
#include <QTemporaryDir>
#include <quazip/quazipfile.h>

class DocReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        createSimpleDocx();
        createDocxWithCourierNew();
        createCorruptedDocx();
        createEmptyDocx();
    }

    void TearDown() override {
    }

    void createSimpleDocx() {
        QBuffer buffer(&simpleDocx);
        buffer.open(QIODevice::WriteOnly);

        QuaZip zip(&buffer);
        zip.open(QuaZip::mdCreate);

        QuaZipFile documentFile(&zip);
        documentFile.open(QIODevice::WriteOnly, QuaZipNewInfo("word/document.xml"));
        documentFile.write(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
            "<w:body><w:p><w:r><w:t>Hello World</w:t></w:r></w:p></w:body>"
            "</w:document>"
        );
        documentFile.close();

        zip.close();
        buffer.close();
    }

    void createDocxWithCourierNew() {
        QBuffer buffer(&courierNewDocx);
        buffer.open(QIODevice::WriteOnly);

        QuaZip zip(&buffer);
        zip.open(QuaZip::mdCreate);

        QuaZipFile documentFile(&zip);
        documentFile.open(QIODevice::WriteOnly, QuaZipNewInfo("word/document.xml"));
        documentFile.write(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
            "<w:body>"
            "<w:p><w:r><w:rPr><w:rFonts w:cs=\"Courier New\"/></w:rPr><w:t>Ignore this</w:t></w:r></w:p>"
            "<w:p><w:r><w:t>Read this</w:t></w:r></w:p>"
            "</w:body>"
            "</w:document>"
        );
        documentFile.close();

        zip.close();
        buffer.close();
    }

    void createCorruptedDocx() {
        QBuffer buffer(&corruptedDocx);
        buffer.open(QIODevice::WriteOnly);

        QuaZip zip(&buffer);
        zip.open(QuaZip::mdCreate);

        QuaZipFile dummyFile(&zip);
        dummyFile.open(QIODevice::WriteOnly, QuaZipNewInfo("lol.txt"));
        dummyFile.write("Is joke?");
        dummyFile.close();

        zip.close();
        buffer.close();
    }

    void createEmptyDocx() {
        QBuffer buffer(&emptyDocx);
        buffer.open(QIODevice::WriteOnly);

        QuaZip zip(&buffer);
        zip.open(QuaZip::mdCreate);

        QuaZipFile documentFile(&zip);
        documentFile.open(QIODevice::WriteOnly, QuaZipNewInfo("word/document.xml"));
        documentFile.write(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
            "<w:body></w:body>"
            "</w:document>"
        );
        documentFile.close();

        zip.close();
        buffer.close();
    }

    QByteArray simpleDocx;
    QByteArray courierNewDocx;
    QByteArray corruptedDocx;
    QByteArray emptyDocx;
};


TEST_F(DocReaderTest, simpleFile) {
    auto res = DocReader::readFile(simpleDocx);
    ASSERT_TRUE(res.has_value());
    qInfo() << res.value();
    EXPECT_EQ(res.value(), "Hello World");
}

TEST_F(DocReaderTest, SkipsCourierText) {
    auto res = DocReader::readFile(courierNewDocx);
    ASSERT_TRUE(res.has_value());
    qInfo() << res.value();
    EXPECT_EQ(res.value(), "Read this");
}

TEST_F(DocReaderTest, HandlesCorruptedZip) {
    auto res = DocReader::readFile(corruptedDocx);
    ASSERT_FALSE(res.has_value());
}

TEST_F(DocReaderTest, HandleEmptyDoc) {
    auto res = DocReader::readFile(emptyDocx);
    ASSERT_TRUE(res.has_value());
    qInfo() << res.value();
    EXPECT_TRUE(res.value().isEmpty());
}

TEST_F(DocReaderTest, HandleInvalidInput) {
    QByteArray input("Not a zip files");
    auto res = DocReader::readFile(input);
    ASSERT_FALSE(res.has_value());
}

TEST_F(DocReaderTest, HandlesXmlError) {
    QByteArray input;
    QBuffer buffer(&input);
    {
        buffer.open(QIODevice::WriteOnly);
        QuaZip zip(&buffer);
        zip.open(QuaZip::mdCreate);
        QuaZipFile file(&zip);
        file.open(QIODevice::WriteOnly, QuaZipFileInfo("word/document.xml"));
        file.write("Is joke?");
        file.close();
        buffer.close();
    }
    auto res = DocReader::readFile(input);
    ASSERT_FALSE(res.has_value());
}