#include <BoostNetwork/BoostNetworkManager.hpp>

#include <gtest/gtest.h>

TEST(BoostNetvorkServerTest, GetAuthUrl) {
    std::vector<std::string> scopes = {"hell ", "got"};

    Server server(scopes, 8080);
    std::string authUrl = "https://accounts.google.com/o/oauth2/v2/auth?scope=hell got&response_type=code&redirect_uri=http://127.0.0.1:8080&client_id=432535903855-baoigtoh3milri8h727fjtniorbqkacj.apps.googleusercontent.com";
    EXPECT_EQ(authUrl, server.getAuthUrl());
}