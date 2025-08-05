#pragma once

#include <string>
#include <unordered_set>
#include <vector>

class Lemmatizer {
public:
    Lemmatizer();
    std::vector<std::string> getLemmas(const std::string &text) const;
    ~Lemmatizer() = default;

private:
    std::unordered_set<std::string> stopWords;
};
