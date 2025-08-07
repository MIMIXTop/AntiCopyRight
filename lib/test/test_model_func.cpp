#include <gtest/gtest.h>

#include <TextAnalyzer/Model/Doc2VecModel.hpp>

class Doc2ModelTesting : public ::testing::Test {
protected:
    void SetUp() override {
        vec1 = torch::tensor({1.0, 0.0, 0.0});
        vec2 = torch::tensor({0.0, 1.0, 0.0});
        vec3 = torch::tensor({1.0, 1.0, 0.0});
        empty = torch::Tensor();
    }
    torch::Tensor vec1, vec2, vec3, empty;
};

TEST_F(Doc2ModelTesting, OrthogonalVectors) {
    double sim = Model::Doc2VecModel::Similarity(vec1, vec2);
    EXPECT_DOUBLE_EQ(sim, 0.0);
}

TEST_F(Doc2ModelTesting, IdenticalVectors) {
    double sim = Model::Doc2VecModel::Similarity(vec1, vec1);
    EXPECT_DOUBLE_EQ(sim, 1.0);
}

TEST_F(Doc2ModelTesting, NonOrthogonalVectors) {
    double sim = Model::Doc2VecModel::Similarity(vec1, vec3);
    double expect = 1.0 / std::sqrt(2.0);
    EXPECT_NEAR(expect, sim, 1e-6);
}

TEST_F(Doc2ModelTesting, EmptiVectors) {
    EXPECT_THROW(Model::Doc2VecModel::Similarity(vec1, empty), std::runtime_error);
    EXPECT_THROW(Model::Doc2VecModel::Similarity(empty, vec1), std::runtime_error);
    EXPECT_THROW(Model::Doc2VecModel::Similarity(empty, empty), std::runtime_error);
}