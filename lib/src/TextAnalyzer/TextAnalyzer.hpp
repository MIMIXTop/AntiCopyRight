#pragma once

#include "Lemmatizer.hpp"

#include <string>

class TextAnalyzer {
    TextAnalyzer(const std::string &dictPath);

    ~TextAnalyzer() = default;

private:
    Lemmatizer lemmatizer;

};

