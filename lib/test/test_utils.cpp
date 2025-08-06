#include <Util/util.hpp>
#include <string>
#include <gtest/gtest.h>

TEST(StringCleaner, NeedClear) {
    std::string testString = "abc\n";
    std::string res = util::StringWorker::getFirstLemma(testString);
    EXPECT_EQ(res, "abc");
}

TEST(StringCleaner, NeedMultiClear) {
    std::string testString = "abc|xyz\n";
    std::string res = util::StringWorker::getFirstLemma(testString);
    EXPECT_EQ(res, "abc");
}