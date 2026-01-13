#include "Network/AuthenticationManager.hpp"
#include "Network/ClassroomManager.hpp"
#include "Network/ReplyType.hpp"
#include "Network/Session.hpp"
#include "Util/util.hpp"
#include "gmock/gmock.h"
#include <algorithm>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/json/serialize.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <fstream>
#include <utility>
#include <variant>
#include <vector>

namespace http = boost::beast::http;
namespace asio = boost::asio;

class MokSession : public Network::Session {
public:
    MokSession(boost::asio::io_context& ioc) : Network::Session(ioc) {}

    MOCK_METHOD(
        (asio::awaitable<http::response<http::dynamic_body>>), sendRequest, (http::request<http::empty_body>),
        (override));

    MOCK_METHOD((boost::asio::awaitable<void>), connectToSender, (const std::string& host), (override));

    MOCK_METHOD((boost::asio::awaitable<void>), stopConnectToSender, (), (override));
};

class MockAuthManager : public Network::AuthenticationManager {
public:
    MOCK_METHOD(bool, EmptyAccessToken, (), (const, override));
    MOCK_METHOD(std::string_view, getToken, (), (const, override));
};

template <typename T>
asio::awaitable<T> ReturnAwaitable(T value) {
    co_return value;
}

asio::awaitable<void> ReturnVoidAwaitable() { co_return; }

class ClassRoomManagerTest : public testing::Test {
protected:
    asio::io_context test_io;
    std::shared_ptr<Network::ClassroomManager> manager;

    MokSession* rawClassRoomSession;
    MokSession* rawDriveSession;
    MockAuthManager* rawAuthManager;

    void SetUp() override {
        auto cls = std::make_unique<testing::NiceMock<MokSession>>(test_io);
        auto drv = std::make_unique<testing::NiceMock<MokSession>>(test_io);
        auto auth = std::make_unique<testing::NiceMock<MockAuthManager>>();

        rawClassRoomSession = cls.get();
        rawDriveSession = drv.get();
        rawAuthManager = auth.get();

        ON_CALL(*rawAuthManager, EmptyAccessToken()).WillByDefault(testing::Return(false));
        ON_CALL(*rawAuthManager, getToken()).WillByDefault(testing::Return("test_token"));

        ON_CALL(*rawClassRoomSession, connectToSender(testing::_))
            .WillByDefault(testing::InvokeWithoutArgs(ReturnVoidAwaitable));
        ON_CALL(*rawDriveSession, connectToSender(testing::_))
            .WillByDefault(testing::InvokeWithoutArgs(ReturnVoidAwaitable));

        manager = std::make_shared<Network::ClassroomManager>(std::move(cls), std::move(drv), std::move(auth));
    }

    void TearDown() override {}
};

TEST_F(ClassRoomManagerTest, GetCourses_Success) {
    test_io.restart();

    std::string json = R"({
            "courses": [
                {"id": "100", "name": "Course 1"},
                {"id": "200", "name": "Course 2"}
            ]
        })";

    EXPECT_CALL(*rawClassRoomSession, connectToSender(testing::_))
        .WillRepeatedly(testing::InvokeWithoutArgs(ReturnVoidAwaitable));

    EXPECT_CALL(*rawClassRoomSession, sendRequest(testing::_)).WillOnce(testing::InvokeWithoutArgs([json]() {
        http::response<http::dynamic_body> res;
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        boost::beast::ostream(res.body()) << json;
        return ReturnAwaitable(std::move(res));
    }));

    std::vector<std::pair<std::string, std::string>> results_pars;
    manager->getCourses([&](ReplyTypes::Reply reply) {
        std::visit(
            util::match {
              [&](ReplyTypes::Types::Courses reply) {
                   for (auto course : reply.courseList) {
                       results_pars.push_back({ course.courseId, course.courseName });
                   }
               },
              [&](ReplyTypes::Types::Error reply) { results_pars.push_back({ "1", reply.errorMessage }); },
              [&](auto replay) {} },
            reply);
    });

    test_io.run();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(results_pars.size(), 2);
    ASSERT_EQ(results_pars[0].first, "100");
    ASSERT_EQ(results_pars[0].second, "Course 1");
    ASSERT_EQ(results_pars[1].first, "200");
    ASSERT_EQ(results_pars[1].second, "Course 2");
    test_io.stop();
}

TEST_F(ClassRoomManagerTest, GetCourseWorks) {
    test_io.restart();

    std::string json = R"({
    "courseWork": [
        {
            "description": "Description 1",
            "courseId": "101",
            "title": "Title 1",
            "id": "10"
        },
        {
            "description": "Description 2",
            "courseId": "101",
            "title": "Title 2",
            "id": "20"
        }
    ]
})";

    EXPECT_CALL(*rawClassRoomSession, connectToSender(testing::_))
        .WillRepeatedly(testing::InvokeWithoutArgs(ReturnVoidAwaitable));

    EXPECT_CALL(*rawClassRoomSession, sendRequest(testing::_)).WillOnce(testing::InvokeWithoutArgs([json]() {
        http::response<http::dynamic_body> res;
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        boost::beast::ostream(res.body()) << json;
        return ReturnAwaitable(std::move(res));
    }));
    std::vector<ReplyTypes::Types::CourseWorks::CourseWork> result;
    manager->getListCoursesWorks("101", [&result](ReplyTypes::Reply reply) {
        std::visit(
            util::match {
              [&](ReplyTypes::Types::CourseWorks courseWorks) {
                  std::for_each(
                      courseWorks.courseWorkList.begin(), courseWorks.courseWorkList.end(),
                      [&result](auto&& obj) { result.push_back(std::move(obj)); });
              },
              [&](ReplyTypes::Types::Error error) {},
              [](auto a) {},
            },
            reply);
    });
    test_io.run();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0).id, "10");
    EXPECT_EQ(result.at(0).courseId, "101");
    EXPECT_EQ(result.at(0).title, "Title 1");
    EXPECT_EQ(result.at(0).description, "Description 1");

    EXPECT_EQ(result.at(1).id, "20");
    EXPECT_EQ(result.at(1).courseId, "101");
    EXPECT_EQ(result.at(1).title, "Title 2");
    EXPECT_EQ(result.at(1).description, "Description 2");

    test_io.stop();
}

TEST_F(ClassRoomManagerTest, GetStudentList_Success) {
    test_io.restart();

    std::string json = R"({
        "students": [
            {
                "courseId": "101",
                "profile": {
                    "photoUrl": "http://example.com/photo1",
                    "emailAddress": "student1@gmail.com",
                    "id": "111",
                    "name": {
                        "fullName": "Student One"
                    }
                },
                "userId": "111"
            },
            {
                "courseId": "101",
                "profile": {
                    "photoUrl": "http://example.com/photo2",
                    "emailAddress": "student2@gmail.com",
                    "id": "222",
                    "name": {
                        "fullName": "Student Two"
                    }
                },
                "userId": "222"
            }
        ]
    })";

    EXPECT_CALL(*rawClassRoomSession, connectToSender(testing::_))
        .WillRepeatedly(testing::InvokeWithoutArgs(ReturnVoidAwaitable));

    EXPECT_CALL(*rawClassRoomSession, sendRequest(testing::_)).WillOnce(testing::InvokeWithoutArgs([json]() {
        http::response<http::dynamic_body> res;
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        boost::beast::ostream(res.body()) << json;
        return ReturnAwaitable(std::move(res));
    }));

    std::vector<ReplyTypes::Types::StudentList::Student> result;
    manager->getStudentsList("101", [&result](ReplyTypes::Reply reply) {
        std::visit(
            util::match {
              [&](ReplyTypes::Types::StudentList studentList) { result = std::move(studentList.studentsList); },
              [&](ReplyTypes::Types::Error error) {},
              [](auto a) {},
            },
            reply);
    });

    test_io.run();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0).userId, "111");
    EXPECT_EQ(result.at(0).fullName, "Student One");
    EXPECT_EQ(result.at(0).email, "student1@gmail.com");
    EXPECT_EQ(result.at(0).photoUrl, "http://example.com/photo1");

    EXPECT_EQ(result.at(1).userId, "222");
    EXPECT_EQ(result.at(1).fullName, "Student Two");
    EXPECT_EQ(result.at(1).email, "student2@gmail.com");
    EXPECT_EQ(result.at(1).photoUrl, "http://example.com/photo2");

    test_io.stop();
}

TEST_F(ClassRoomManagerTest, GetUserInfo_Success) {
    test_io.restart();

    std::string json = R"({
        "family_name": "Doe",
        "name": "John Doe",
        "picture": "http://example.com/john.jpg",
        "email": "johndoe@gmail.com",
        "given_name": "John",
        "id": "999",
        "verified_email": true
    })";

    EXPECT_CALL(*rawDriveSession, connectToSender(testing::_))
        .WillRepeatedly(testing::InvokeWithoutArgs(ReturnVoidAwaitable));

    EXPECT_CALL(*rawDriveSession, sendRequest(testing::_)).WillOnce(testing::InvokeWithoutArgs([json]() {
        http::response<http::dynamic_body> res;
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        boost::beast::ostream(res.body()) << json;
        return ReturnAwaitable(std::move(res));
    }));

    ReplyTypes::Types::UserInfo result;
    manager->getUserInfo([&result](ReplyTypes::Reply reply) {
        std::visit(
            util::match {
              [&](ReplyTypes::Types::UserInfo userInfo) { result = std::move(userInfo); },
              [&](ReplyTypes::Types::Error error) {},
              [](auto a) {},
            },
            reply);
    });

    test_io.run();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_EQ(result.id, "999");
    EXPECT_EQ(result.name, "John Doe");
    EXPECT_EQ(result.photoUrl, "http://example.com/john.jpg");
    EXPECT_EQ(result.email, "johndoe@gmail.com");

    test_io.stop();
}

TEST_F(ClassRoomManagerTest, GetStudentsWorks_Success) {
    test_io.restart();

    std::string json = R"({
        "studentSubmissions": [
            {
                "courseId": "100",
                "courseWorkId": "10",
                "id": "sub1",
                "userId": "200",
                "assignmentSubmission": {
                    "attachments": [
                        {
                            "driveFile": {
                                "id": "file1",
                                "title": "test.pdf",
                                "alternateLink": "http://link",
                                "thumbnailUrl": "http://thumb"
                            }
                        }
                    ]
                }
            }
        ]
    })";

    EXPECT_CALL(*rawClassRoomSession, connectToSender(testing::_))
        .WillRepeatedly(testing::InvokeWithoutArgs(ReturnVoidAwaitable));

    EXPECT_CALL(*rawClassRoomSession, sendRequest(testing::_)).WillOnce(testing::InvokeWithoutArgs([json]() {
        http::response<http::dynamic_body> res;
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        boost::beast::ostream(res.body()) << json;
        return ReturnAwaitable(std::move(res));
    }));

    std::vector<ReplyTypes::Types::StudentWorks::StudentWork> result;
    manager->getStudentsWorks("100", "10", [&result](ReplyTypes::Reply reply) {
        std::visit(
            util::match {
              [&](ReplyTypes::Types::StudentWorks studentWorks) { result = std::move(studentWorks.studentWorkList); },
              [&](ReplyTypes::Types::Error error) {},
              [](auto a) {},
            },
            reply);
    });

    test_io.run();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].id, "sub1");
    EXPECT_EQ(result[0].courseId, "100");
    EXPECT_EQ(result[0].courseWorkId, "10");
    EXPECT_EQ(result[0].userId, "200");
    
    ASSERT_EQ(result[0].files.size(), 1);
    EXPECT_EQ(result[0].files[0].fileId, "file1");
    EXPECT_EQ(result[0].files[0].title, "test.pdf");

    test_io.stop();
}

TEST_F(ClassRoomManagerTest, DownloadStudentWork_Success) {
    test_io.restart();

    std::string fileContent = "binary_content";
    std::string fileId = "file1";
    std::string fileName = "file.txt";

    EXPECT_CALL(*rawDriveSession, connectToSender(testing::_))
        .WillRepeatedly(testing::InvokeWithoutArgs(ReturnVoidAwaitable));

    EXPECT_CALL(*rawDriveSession, sendRequest(testing::_)).WillOnce(testing::InvokeWithoutArgs([fileContent]() {
        http::response<http::dynamic_body> res;
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/octet-stream");
        boost::beast::ostream(res.body()) << fileContent;
        return ReturnAwaitable(std::move(res));
    }));

    ReplyTypes::Types::DownloadStudentWork result;
    bool success = false;

    manager->downloadStudentWork(fileName, fileId, [&](ReplyTypes::Reply reply) {
        std::visit(
            util::match {
              [&](ReplyTypes::Types::DownloadStudentWork work) { 
                  result = std::move(work); 
                  success = true;
              },
              [&](ReplyTypes::Types::Error error) {},
              [](auto a) {},
            },
            reply);
    });

    test_io.run();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_TRUE(success);
    EXPECT_EQ(result.fileName, fileName);
    std::string downloadedContent(result.courseWork.begin(), result.courseWork.end());
    EXPECT_EQ(downloadedContent, fileContent);

    test_io.stop();
}
