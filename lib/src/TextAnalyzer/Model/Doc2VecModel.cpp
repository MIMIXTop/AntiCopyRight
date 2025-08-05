//
// Created by mimixtop on 05.08.25.
//

#include "Doc2VecModel.hpp"
#include <torch/torch.h>
#include <QDebug>

namespace {
#ifdef WIN32
    #define PATH_MODEL "Utils\\Model\\doc2vec_model.pt"
    #define PATH_WORD2IDX "Utils\\Model\\word2idx.txt"
#else
    #define PATH_MODEL "Utils/Model/doc2vec_model.pt"
    #define PATH_WORD2IDX "Utils/Model/word2idx.txt"
#endif

    std::unordered_map<std::string, int32_t> getWord2idx(const std::string &path = PATH_WORD2IDX) {
        std::unordered_map<std::string, int32_t> word2idx = {};

        if (std::ifstream file(path); file.is_open()) {
            std::string word;
            int32_t index;
            while (file >> word >> index) {
                word2idx[word] = index;
            }
            file.close();
        } else {
            qInfo() << "Could not open file to path: " << path;
        }

        return word2idx;
    }
}


Doc2VecModel::Doc2VecModel() {
    setDevice();
    model = torch::jit::load("");
}

void Doc2VecModel::setDevice() const {
    if (torch::cuda::is_available()) {
        device = torch::Device(torch::kCUDA);
    } else {
        device = torch::Device(torch::kCPU);
    }
}
