#include <BoostNetwork/AuthenticationManager.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(BoostNetvorkServerTest, GetAuthUrl) {
    std::vector<std::string> scopes = {"hell", "got"};

    AuthenticationManager server(scopes, 8080);
    std::string authUrl = server.getAuthUrl();
    EXPECT_THAT(authUrl, testing::HasSubstr("scope=hell got "));
    EXPECT_THAT(authUrl, testing::HasSubstr("response_type=code"));
    EXPECT_THAT(authUrl, testing::HasSubstr("redirect_uri=http://127.0.0.1:8080/code"));
    EXPECT_THAT(authUrl, testing::HasSubstr("client_id="));
}

/* TEST(BoostNetvorkServerTest, VoidTest) {
    AuthenticationManager server;
    std::cout << server.getAuthUrl() << std::endl;
    std::thread th{[&server] {
       server.run_server();
    }};
    while (server.EmptyAccessToken()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    EXPECT_FALSE(server.EmptyAccessToken());

    th.detach();
} */