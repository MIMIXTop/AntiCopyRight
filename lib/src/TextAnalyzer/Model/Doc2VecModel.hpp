#pragma once

#include <TextAnalyzer/Lemmantizer/Lemmatizer.hpp>

#include <torch/script.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace Model {
    class Doc2VecModel {
    public:
        Doc2VecModel(): device(setDevice()), word2idx(setWord2Idx()) {}

        torch::Tensor getDocVector(const std::string &document) const;

        void loadModel();

        static double Similarity(const torch::Tensor &doc1, const torch::Tensor &doc2);

        ~Doc2VecModel() = default;

    private:
        static torch::Device setDevice();

        static std::unordered_map<std::string, int32_t> setWord2Idx();

        std::unique_ptr<torch::jit::Module> model;
        Lemmatizer lemmatizer;
        torch::Device device;
        std::unordered_map<std::string, int32_t> word2idx;

        std::unique_ptr<torch::jit::Module> word_embeddings;
        at::Tensor weights;
        int32_t embedding_dim = 0;
    };
}
