#include  <gtest/gtest.h>
#include <filesystem>
#include <string>

TEST(MystemMacro, IsDefinedAndNonEmpty) {
    const std::string path = MYSTEM_EXECUTABLE;
    EXPECT_FALSE(path.empty()) << "MYSTEM_EXECUTABLE задан пустой строкой";
}

TEST(MystemMacro, FileExists) {
    const std::string &path = MYSTEM_EXECUTABLE;
    std::filesystem::path p(path);

    EXPECT_TRUE(std::filesystem::exists(p)) << "Файл MyStem не найден: " << path;
    EXPECT_TRUE(std::filesystem::is_regular_file(p)) << "По пути " << path << " лежит необычный объект";
}