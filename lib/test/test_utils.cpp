#include <Util/util.hpp>
#include <gtest/gtest.h>
#include <string>

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