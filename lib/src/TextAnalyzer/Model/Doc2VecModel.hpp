#pragma once

#include <torch/script.h>
#include <memory>

class Doc2VecModel {
public:
    Doc2VecModel();
    torch::Tensor getDocVector() const;
    ~Doc2VecModel() = default;
private:
    void setDevice() const;

    std::shared_ptr<torch::jit::script::Module> model;
    const torch::Device device;
    std::unordered_map<std::string, int32_t> word2idx;
};
