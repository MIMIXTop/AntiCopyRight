#pragma once

#include <TextAnalyzer/Lemmatizer.hpp>

#include <torch/script.h>
#include <unordered_map>
#include <string>
#include <memory>

class Doc2VecModel {
public:
    Doc2VecModel() {
        setDevice();
        setWord2Idx();
    }

    torch::Tensor getDocVector(const std::string &document) const;
    void loadModel();
    double Similarity(const torch::Tensor &doc1, const torch::Tensor &doc2) const;

    ~Doc2VecModel() = default;
private:
    static void setDevice();
    static void setWord2Idx();

    std::unique_ptr<torch::jit::Module> model;
    Lemmatizer lemmatizer;
    static torch::Device device;
    static std::unordered_map<std::string, int32_t> word2idx;

    std::unique_ptr<torch::jit::Module> word_embeddings;
    at::Tensor weights;
    int32_t embedding_dim = 0;

};
