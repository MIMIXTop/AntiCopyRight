#include "../src/TextAnalyzer/Lemmantizer/Lemmatizer.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(LemmantizerTest, EmptyRequest) {
    EXPECT_EQ(Lemmatizer::getLemmas("").size(), 0);
}

TEST(LemmantizerTest, OneWord) {
    const std::string text = "Привет";
    std::vector lemmas = Lemmatizer::getLemmas(text);

    EXPECT_EQ(lemmas.size(), 1) << "Problems with size";
    EXPECT_EQ(lemmas.at(0), "привет");
}

TEST(LemmantizerTest, OneSentence) {
    const std::string text = "привет, мир 1 2";
    std::vector lemmas = Lemmatizer::getLemmas(text);

    EXPECT_EQ(lemmas.size(), 2);
    EXPECT_EQ(lemmas.at(0), "привет" );
    EXPECT_EQ(lemmas.at(1), "мир" );
}