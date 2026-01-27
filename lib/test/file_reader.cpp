#include "DocumentManager/DocReader.hpp"
#include <gtest/gtest.h>
#include <mz.h>
#include <mz_strm.h>
#include <mz_strm_mem.h>
#include <mz_zip.h>
#include <vector>

class FileReaderTest : public testing::Test {
private:
    std::vector<unsigned char> createZipInMemory(const std::string& content, const char* entry_name) {
        void* mem_stream = mz_stream_mem_create();
        if (!mem_stream) {
            throw std::runtime_error("Failed to create memory stream for ZIP");
        }

        mz_stream_mem_set_grow_size(mem_stream, 4096);

        if (mz_stream_open(mem_stream, nullptr, MZ_OPEN_MODE_CREATE) != MZ_OK) {
            mz_stream_mem_delete(&mem_stream);
            throw std::runtime_error("Failed to open memory stream for writing");
        }

        void* zip_handle = mz_zip_create();
        if (!zip_handle) {
            mz_stream_close(mem_stream);
            mz_stream_mem_delete(&mem_stream);
            throw std::runtime_error("Failed to create ZIP handle");
        }

        if (mz_zip_open(zip_handle, mem_stream, MZ_OPEN_MODE_WRITE) != MZ_OK) {
            mz_zip_delete(&zip_handle);
            mz_stream_close(mem_stream);
            mz_stream_mem_delete(&mem_stream);
            throw std::runtime_error("Failed to open ZIP for writing");
        }

        mz_zip_file file_info = { 0 };
        file_info.filename = entry_name;
        file_info.modified_date = time(nullptr);
        file_info.uncompressed_size = content.size();
        file_info.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
        file_info.flag = MZ_ZIP_FLAG_UTF8;

        if (mz_zip_entry_write_open(zip_handle, &file_info, MZ_COMPRESS_LEVEL_DEFAULT, 0, nullptr) != MZ_OK) {
            mz_zip_close(zip_handle);
            mz_zip_delete(&zip_handle);
            mz_stream_close(mem_stream);
            mz_stream_mem_delete(&mem_stream);
            throw std::runtime_error("Failed to open ZIP entry for writing");
        }

        int32_t written = mz_zip_entry_write(zip_handle, content.data(), static_cast<int32_t>(content.size()));
        if (written < 0) {
            mz_zip_entry_close(zip_handle);
            mz_zip_close(zip_handle);
            mz_zip_delete(&zip_handle);
            mz_stream_close(mem_stream);
            mz_stream_mem_delete(&mem_stream);
            throw std::runtime_error("Failed to write content to ZIP entry");
        }

        if (mz_zip_entry_close(zip_handle) != MZ_OK) {
            mz_zip_close(zip_handle);
            mz_zip_delete(&zip_handle);
            mz_stream_close(mem_stream);
            mz_stream_mem_delete(&mem_stream);
            throw std::runtime_error("Failed to close ZIP entry");
        }

        if (mz_zip_close(zip_handle) != MZ_OK) {
            mz_zip_delete(&zip_handle);
            mz_stream_close(mem_stream);
            mz_stream_mem_delete(&mem_stream);
            throw std::runtime_error("Failed to close ZIP archive");
        }

        void* buffer = nullptr;
        int32_t buffer_size = 0;
        if (mz_stream_mem_get_buffer(mem_stream, (const void**) &buffer) != MZ_OK) {
            mz_zip_delete(&zip_handle);
            mz_stream_close(mem_stream);
            mz_stream_mem_delete(&mem_stream);
            throw std::runtime_error("Failed to get ZIP buffer");
        }

        mz_stream_mem_get_buffer_length(mem_stream, &buffer_size);
        const unsigned char* buf_ptr = static_cast<const unsigned char*>(buffer);
        std::vector<unsigned char> result(buf_ptr, buf_ptr + buffer_size);

        mz_zip_delete(&zip_handle);
        mz_stream_close(mem_stream);
        mz_stream_mem_delete(&mem_stream);

        return result;
    }

protected:
    void SetUp() override {
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

        std::string docxCourierNew = R"(
            <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
                <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
                    <w:body>
                    <w:p>
                        <w:r>
                            <w:rPr>
                                <w:rFonts w:cs="Courier New"/>
                            </w:rPr>
                            <w:t>Ignore this</w:t>
                        </w:r>
                    </w:p>
                    <w:p>   
                        <w:r>
                            <w:t>Read this</w:t>
                        </w:r>
                    </w:p>
                </w:body>
            </w:document>
        )";

        std::string docxCorrupted = "Is joke?";

        docxFile = createZipInMemory(docx, "word/document.xml");
        docxRUFile = createZipInMemory(docxRU, "word/document.xml");
        docxCourierNewFile = createZipInMemory(docxCourierNew, "word/document.xml");
        docxCorruptedFile = createZipInMemory(docxCorrupted, "lol.txt");
    }

    void TearDown() override {}

    std::vector<unsigned char> docxFile;
    std::vector<unsigned char> docxRUFile;
    std::vector<unsigned char> docxCourierNewFile;
    std::vector<unsigned char> docxCorruptedFile;
};

TEST_F(FileReaderTest, zipReader) {
    auto result = DocReader::zipReader(this->docxFile);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), "Hello World");
}

TEST_F(FileReaderTest, zipReaderRU) {
    auto result = DocReader::zipReader(this->docxRUFile);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), "Привет мир");
}

TEST_F(FileReaderTest, zipReaderCourierNew) {
    auto result = DocReader::zipReader(this->docxCourierNewFile);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), "Read this");
}

TEST_F(FileReaderTest, zipReaderCorrupted) {
    auto result = DocReader::zipReader(this->docxCorruptedFile);
    ASSERT_FALSE(result.has_value());
}
