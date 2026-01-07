#include <BoostNetwork/AuthenticationManager.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(BoostNetvorkServerTest, GetAuthUrl) {
  std::vector<std::string> scopes = {"hell", "got"};

  Network::AuthenticationManager server(scopes, 8080);
  std::string authUrl = server.getAuthUrl();
  EXPECT_THAT(authUrl, testing::HasSubstr("scope=hell got "));
  EXPECT_THAT(authUrl, testing::HasSubstr("response_type=code"));
  EXPECT_THAT(authUrl,
              testing::HasSubstr("redirect_uri=http://127.0.0.1:8080/code"));
  EXPECT_THAT(authUrl, testing::HasSubstr("client_id="));
}