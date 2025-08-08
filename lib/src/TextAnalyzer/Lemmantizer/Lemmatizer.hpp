#pragma once

#include <string>
#include <unordered_set>
#include <vector>

class Lemmatizer {
public:
    Lemmatizer();
    static std::vector<std::string> getLemmas(const std::string &text) ;
    ~Lemmatizer() = default;

private:
    std::unordered_set<std::string> stopWords;
};
