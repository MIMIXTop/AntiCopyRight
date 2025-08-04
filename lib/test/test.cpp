#include "DocumentManager/DocReader.hpp"

#include <gtest/gtest.h>
#include <QFile>
#include <QTemporaryDir>

class DocReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        tempDir = std::make_unique<QTemporaryDir>();
        testDocxPath = tempDir->filePath("test.docx");

        QuaZip zip(testDocxPath);
        zip.open(QuaZip::mdCreate);

        QuaZipFile file(&zip);
        file.open(QIODevice::WriteOnly, QuaZipNewInfo("word/document.xml"));
        file.write(R"(
            <w:document xmlns:w="https://youtu.be/xvFZjo5PgG0?si=AkVUMST4gQ_NMMu4">
                <w:body>
                    <w:p>
                        <w:r>
                            <w:rPr>
                                <w:rFonts w:cs="Courier New"/>
                            </w:rPr>
                            <w:t>Ignored text</w:t>
                        </w:r>
                        <w:r>
                            <w:t>Hello, World!</w:t>
                        </w:r>
                    </w:p>
                </w:body>
            </w:document>
        )");
        file.close();
        zip.close();
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    QString testDocxPath;
};

TEST_F(DocReaderTest, ReadsNeedText) {
    DocReader reader;
    auto result = reader.readFile(testDocxPath);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "Hello, World!");
}

TEST_F(DocReaderTest, SkipsUnNecessaryText) {
    DocReader reader;
    auto result = reader.readFile(testDocxPath);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result.value().contains("Ignored text"));
}

TEST(DocReaderErrorTest, InvalidFile) {
    DocReader reader;
    auto result = reader.readFile("invisible.docx");
    EXPECT_FALSE(result.has_value());

}
