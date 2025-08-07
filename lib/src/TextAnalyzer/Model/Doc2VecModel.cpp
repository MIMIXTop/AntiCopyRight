#include "Doc2VecModel.hpp"

#include <torch/torch.h>
#include <torch/script.h>
#include <vector>
#include <string_view>
#include <QDebug>

namespace {
#ifdef WIN32
    #define PATH_MODEL "Utils\\Model\\doc2vec_model.pt"
    #define PATH_WORD2IDX "Utils\\Model\\word2idx.txt"
#else
    #define PATH_MODEL "Utils/Model/doc2vec_model.pt"
    #define PATH_WORD2IDX "Utils/Model/word2idx.txt"
#endif
}

torch::Tensor Model::Doc2VecModel::getDocVector(const std::string &document) const {
    if (model == nullptr) {
        throw std::runtime_error("To use this you must first load the model");
    }

    const std::vector tokens = Lemmatizer::getLemmas(document);
    std::vector<int32_t> indices;

    for (auto&& token : tokens) {
        if (word2idx.contains(token)) {
            indices.emplace_back(word2idx.at(token));
        }
    }

    if (indices.empty()) {
        return torch::zeros({embedding_dim}).to(device);
    }

    at::Tensor input = torch::from_blob(indices.data(), indices.size(), torch::TensorOptions()).to(device);
    auto embeddings = word_embeddings->forward({input}).toTensor();

    if (embeddings.size(0) == 0) {
        return torch::zeros({embedding_dim}).to(device);
    }

    return embeddings.mean(0);
}

void Model::Doc2VecModel::loadModel() {
    try {
        model = std::make_unique<torch::jit::Module>( torch::jit::load(PATH_MODEL));
        model->to(device);
        model->eval();
        qInfo() << "Model loaded";
    } catch (const c10::Error& e) {
        qInfo() << "Falid load model" << e.what();
    }

    word_embeddings = std::make_unique<torch::jit::Module>(model->attr("word_embed").toModule());
    weights = model->attr("weight").toTensor();
    embedding_dim = weights.size(1);
}

double Model::Doc2VecModel::Similarity(const torch::Tensor &doc1, const torch::Tensor &doc2) {
    if (doc1.size(0) == 0 || doc2.size(0) == 0) {
        throw std::runtime_error("one of the vectors is empty");
    }

    return torch::nn::functional::cosine_similarity(
               doc1.unsqueeze(0), doc2.unsqueeze(0),
               torch::nn::functional::CosineSimilarityFuncOptions().dim(1)
               ).item<double>();
}

torch::Device Model::Doc2VecModel::setDevice() {
    return  torch::Device(torch::cuda::is_available() ? torch::kCUDA : torch::kCPU);
}

std::unordered_map<std::string, int32_t> Model::Doc2VecModel::setWord2Idx() {
    std::unordered_map<std::string, int32_t> word2idx_copy;
    if (std::ifstream file(PATH_WORD2IDX); file.is_open()) {
        std::string word;
        int32_t index;
        while (file >> word >> index) {
            word2idx_copy[word] = index;
        }
        file.close();
    } else {
        throw std::runtime_error("Could not open file to path: " + std::string(PATH_WORD2IDX));
    }

    return word2idx_copy;
}
