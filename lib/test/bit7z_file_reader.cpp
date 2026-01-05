#include "DocumentManager/DocReader.hpp"
#include "bit7z/bit7zlibrary.hpp"
#include "bit7z/bitcompressor.hpp"
#include "bit7z/bitformat.hpp"
#include <bit7z/bitfilecompressor.hpp>
#include <gtest/gtest.h>
#include <vector>

class Bit7zFileReaderTest : public testing::Test {
protected:
    void SetUp() override {
        createDocx();
    }

    void TearDown() override {
    }

    void createDocx() {
        bit7z::Bit7zLibrary lib{DocReader::get7zLibraryPath()};

        std::string docx = R"(
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
                <w:body>
                    <w:p>
                        <w:r>
                            <w:t>Hello World</w:t>
                        </w:r>
                    </w:p>
                </w:body>
            </w:document>
        )";

        std::string docxRU = R"(
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
                <w:body>
                    <w:p>
                        <w:r>
                            <w:t>Привет мир</w:t>
                        </w:r>
                    </w:p>
                </w:body>
            </w:document>
        )";

        std::vector<unsigned char> fileData;
        fileData.assign(docx.begin(), docx.end());
        bit7z::BitCompressor<std::vector<unsigned char>> compressor{lib,  bit7z::BitFormat::Zip};
        compressor.compressFile(fileData, this->docx, "word/document.xml");

        fileData.assign(docxRU.begin(), docxRU.end());
        compressor.compressFile(fileData, this->docxRU, "word/document.xml");
    }

    std::vector<unsigned char> docx;
    std::vector<unsigned char> docxRU;
};

TEST_F(Bit7zFileReaderTest, readFile) {
    auto result = DocReader::readFile(std::move(this->docx));
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), "Hello World");
}

TEST_F(Bit7zFileReaderTest, readFileRU) {
    auto result = DocReader::readFile(std::move(this->docxRU));
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), "Привет мир");
}
