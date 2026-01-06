#include "BoostNetwork/AuthenticationManager.hpp"
#include "BoostNetwork/ClassroomManager.hpp"
#include "BoostNetwork/ReplyType.hpp"
#include "BoostNetwork/Session.hpp"
#include "Util/util.hpp"
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/json/serialize.hpp>
#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace http = boost::beast::http;
namespace asio = boost::asio;

class MokSession : public Network::Session {
public:
  MokSession(boost::asio::io_context &ioc) : Network::Session(ioc) {}

  MOCK_METHOD((asio::awaitable<http::response<http::dynamic_body>>),
              sendRequest, (http::request<http::empty_body>), (override));

  MOCK_METHOD((boost::asio::awaitable<void>), connectToSender,
              (const std::string &host), (override));

  MOCK_METHOD((boost::asio::awaitable<void>), stopConnectToSender, (),
              (override));
};

class MockAuthManager : public Network::AuthenticationManager {
public:
  MOCK_METHOD(bool, EmptyAccessToken, (), (const, override));
  MOCK_METHOD(std::string_view, getToken, (), (const, override));
};

template <typename T> asio::awaitable<T> ReturnAwaitable(T value) {
  co_return value;
}

asio::awaitable<void> ReturnVoidAwaitable() { co_return; }

class ClassRoomManagerTest : public testing::Test {
protected:
  asio::io_context test_io;
  std::shared_ptr<Network::ClassroomManager> manager;

  MokSession *rawClassRoomSession;
  MokSession *rawDriveSession;
  MockAuthManager *rawAuthManager;

  void SetUp() override {
    auto cls = std::make_unique<testing::NiceMock<MokSession>>(test_io);
    auto drv = std::make_unique<testing::NiceMock<MokSession>>(test_io);
    auto auth = std::make_unique<testing::NiceMock<MockAuthManager>>();

    rawClassRoomSession = cls.get();
    rawDriveSession = drv.get();
    rawAuthManager = auth.get();

    ON_CALL(*rawAuthManager, EmptyAccessToken())
        .WillByDefault(testing::Return(false));
    ON_CALL(*rawAuthManager, getToken())
        .WillByDefault(testing::Return("test_token"));

    ON_CALL(*rawClassRoomSession, connectToSender(testing::_))
        .WillByDefault(testing::InvokeWithoutArgs(ReturnVoidAwaitable));
    ON_CALL(*rawDriveSession, connectToSender(testing::_))
        .WillByDefault(testing::InvokeWithoutArgs(ReturnVoidAwaitable));

    manager = std::make_shared<Network::ClassroomManager>(
        std::move(cls), std::move(drv), std::move(auth));
  }

  void TearDown() override {}
};

TEST_F(ClassRoomManagerTest, GetCourses_Success) {
  test_io.restart();

  std::string json = R"({
			"courses": [
				{"id": 1, "name": "C++"},
				{"id": 2, "name": "NoPython"}
			]
		})";

  EXPECT_CALL(*rawClassRoomSession, connectToSender(testing::_))
      .WillRepeatedly(testing::InvokeWithoutArgs(ReturnVoidAwaitable));

  EXPECT_CALL(*rawClassRoomSession, sendRequest(testing::_))
      .WillOnce(testing::InvokeWithoutArgs([json]() {
        http::response<http::dynamic_body> res;
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        boost::beast::ostream(res.body()) << json;
        return ReturnAwaitable(std::move(res));
      }));

  std::vector<std::pair<int, std::string>> results_pars;
  manager->getCourses([&](ReplyTypes::BoostReply reply) {
    std::visit(
        util::match{[&](ReplyTypes::BoostTypes::Course reply) {
                      for (std::ptrdiff_t i = 0; i < reply.course.size(); i++) {
                        auto item = reply.course.at(i).as_object();
                        results_pars.push_back(
                            {static_cast<int>(item.at("id").as_int64()),
                             item.at("name").as_string().c_str()});
                      }
                    },
                    [&](ReplyTypes::BoostTypes::Error reply) {
                      results_pars.push_back(
                          {1, reply.error.at("message").as_string().c_str()});
                    },
                    [&](auto reply) {
                      results_pars.push_back({1, "Unknown reply type"});
                    }},
        reply);
  });

  test_io.run();
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(results_pars.size(), 2);
  ASSERT_EQ(results_pars[0].first, 1);
  ASSERT_EQ(results_pars[0].second, "C++");
  ASSERT_EQ(results_pars[1].first, 2);
  ASSERT_EQ(results_pars[1].second, "NoPython");
  test_io.stop();
}