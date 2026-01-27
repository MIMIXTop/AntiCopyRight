#include "ClassroomManager.hpp"

#include <algorithm>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serializer.hpp>
#include <chrono>
#include <format>
#include <future>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include <Network/ReplyType.hpp>
#include <Network/RequestDTO.hpp>

#include "Util/util.hpp"

namespace {
#define GOOGLE_CLASSROOM_HOST "classroom.googleapis.com"
#define GOOGLE_HOST           "www.googleapis.com"

#define GET_FIELD(object, field) object.as_object().at(#field).as_string()

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
}   // namespace

namespace Network {
ClassroomManager::ClassroomManager() {
    networkThread = std::jthread([this] {
        asio::signal_set signals { ioContext_, SIGINT, SIGTERM };
        signals.async_wait([this](auto, auto) { ioContext_.stop(); });
        ioContext_.run();
    });

    classroomSession_ = std::make_unique<Session>(ioContext_);
    asio::co_spawn(ioContext_, classroomSession_->connectToSender(GOOGLE_CLASSROOM_HOST), asio::detached);

    googleSession_ = std::make_unique<Session>(ioContext_);
    asio::co_spawn(ioContext_, googleSession_->connectToSender(GOOGLE_HOST), asio::detached);

    authenticationManager_ = std::make_unique<AuthenticationManager>();
    authThread = std::jthread([this]() { authenticationManager_->run_server(); });
}
ClassroomManager::ClassroomManager(
    std::unique_ptr<Session> ClassSess, std::unique_ptr<Session> GoogleSess,
    std::unique_ptr<AuthenticationManager> AuthMan)
  : classroomSession_(std::move(ClassSess))
  , googleSession_(std::move(GoogleSess))
  , authenticationManager_(std::move(AuthMan)) {
    networkThread = std::jthread([this] {
        auto work = asio::make_work_guard(ioContext_);
        ioContext_.run();
    });

    authThread = std::jthread([this]() { authenticationManager_->run_server(); });
}

ClassroomManager::~ClassroomManager() { ioContext_.stop(); }

void ClassroomManager::getCourses(HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func)]() -> asio::awaitable<void> {
            auto request = requestHandler(DTOCourseList {});
            if (request.method() == http::verb::unknown) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = "No access token available. Please authenticate first.";
                handler(obj);
                co_return;
            }
            http::response<http::dynamic_body> res = co_await classroomSession_->sendRequest(request);

            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();
            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = std::move(beast::buffers_to_string(res.body().cdata()));
                handler(obj);
                co_return;
            }

            ReplyTypes::Types::Courses obj;
            auto tempCourseList = jsonBody.at("courses").as_array();
            for (const auto& course : tempCourseList) {
                obj.courseList.emplace_back(GET_FIELD(course, id).c_str(), GET_FIELD(course, name).c_str());
            }
            handler(obj);
        },
        asio::detached);
}

void ClassroomManager::getListCoursesWorks(const std::string& courseId, HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func), courseId]() -> asio::awaitable<void> {
            auto request = requestHandler(DTOCourseWorksList { courseId });
            if (request.method() == http::verb::unknown) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = "No access token available. Please authenticate first.";
                handler(obj);
                co_return;
            }
            http::response<http::dynamic_body> res = co_await classroomSession_->sendRequest(request);

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = std::move(beast::buffers_to_string(res.body().cdata()));
                handler(obj);
                co_return;
            }
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();

            ReplyTypes::Types::CourseWorks obj;

            auto tempObjectList = jsonBody.at("courseWork").as_array();
            for (auto&& courseWork : tempObjectList) {
                obj.courseWorkList.emplace_back(
                    GET_FIELD(courseWork, id).c_str(), GET_FIELD(courseWork, courseId).c_str(),
                    GET_FIELD(courseWork, title).c_str(), GET_FIELD(courseWork, description).c_str());
            }
            handler(obj);
            co_return;
        },
        asio::detached);
}

void ClassroomManager::getStudentsWorks(
    const std::string& courseId, const std::string& courseWorkId, HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func), &courseId, &courseWorkId]() -> asio::awaitable<void> {
            auto request = requestHandler(DTOStudentWorksData { .courseId = courseId, .courseWorkId = courseWorkId });
            if (request.method() == http::verb::unknown) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = "No access token available. Please authenticate first.";
                handler(obj);
                co_return;
            }
            http::response<http::dynamic_body> res = co_await classroomSession_->sendRequest(request);

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = std::move(beast::buffers_to_string(res.body().cdata()));
                handler(obj);
                co_return;
            }
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();

            ReplyTypes::Types::StudentWorks obj;

            for (auto&& studentSubmission : jsonBody.at("studentSubmissions").as_array()) {
                ReplyTypes::Types::StudentWorks::StudentWork temp;

                temp.id = std::move(GET_FIELD(studentSubmission, id));
                temp.courseWorkId = std::move(GET_FIELD(studentSubmission, courseWorkId));
                temp.courseId = std::move(GET_FIELD(studentSubmission, courseId));
                temp.userId = std::move(GET_FIELD(studentSubmission, userId));

                for (
                    auto&& files :
                    studentSubmission.as_object().at("assignmentSubmission").as_object().at("attachments").as_array()) {
                    auto driveFiles = files.as_object().at("driveFile");

                    temp.files.emplace_back(
                        GET_FIELD(driveFiles, id).c_str(), GET_FIELD(driveFiles, title).c_str(),
                        GET_FIELD(driveFiles, alternateLink).c_str(), GET_FIELD(driveFiles, thumbnailUrl).c_str());
                }
                obj.studentWorkList.push_back(std::move(temp));
            }
            handler(obj);
            co_return;
        },
        asio::detached);
}

void ClassroomManager::getStudentsList(const std::string& courseId, HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func), courseId]() -> asio::awaitable<void> {
            auto request = requestHandler(DTOStudentsList { courseId });
            if (request.method() == http::verb::unknown) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = "No access token available. Please authenticate first.";
                handler(obj);
                co_return;
            }
            http::response<http::dynamic_body> res = co_await classroomSession_->sendRequest(request);

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = std::move(beast::buffers_to_string(res.body().cdata()));
                handler(obj);
                co_return;
            }
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();

            ReplyTypes::Types::StudentList obj;
            for (auto&& studentValue : jsonBody.at("students").as_array()) {
                obj.courseId = std::move(GET_FIELD(studentValue, courseId));
                auto&& profil = studentValue.as_object().at("profile");

                obj.studentsList.emplace_back(
                    GET_FIELD(profil, id).c_str(), profil.at("name").as_object().at("fullName").as_string().c_str(),
                    GET_FIELD(profil, emailAddress).c_str(), GET_FIELD(profil, photoUrl).c_str());
            }
            handler(obj);
        },
        asio::detached);
}

void ClassroomManager::downloadStudentWork(
    const std::string& fileName, const std::string& fileId, HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func), fileName, fileId]() -> asio::awaitable<void> {
            auto request = requestHandler(DTOStudentWorksDownload { fileId });
            if (request.method() == http::verb::unknown) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = "No access token available. Please authenticate first.";
                handler(obj);
                co_return;
            }
            http::response<http::dynamic_body> res = co_await googleSession_->sendRequest(request);

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = std::move(beast::buffers_to_string(res.body().cdata()));
                handler(obj);

                co_return;
            }

            auto bodyBegin = asio::buffers_begin(res.body().cdata());
            auto bodyEnd = asio::buffers_end(res.body().cdata());
            ReplyTypes::Types::DownloadStudentWork obj;
            obj.courseWork = std::vector<uint8_t> { bodyBegin, bodyEnd };
            obj.fileName = fileName;
            handler(obj);
        },
        asio::detached);
}

void ClassroomManager::getUserInfo(HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func)]() -> asio::awaitable<void> {
            auto request = requestHandler(DTOUserInfo {});
            if (request.method() == http::verb::unknown) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = "No access token available. Please authenticate first.";
                handler(obj);
                co_return;
            }
            http::response<http::dynamic_body> res = co_await googleSession_->sendRequest(request);

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = std::move(beast::buffers_to_string(res.body().cdata()));
                handler(obj);
                co_return;
            }
            auto body = beast::buffers_to_string(res.body().cdata());
            try {
                auto jsonBody = json::parse(body).as_object();

                ReplyTypes::Types::UserInfo uInf;
                if (jsonBody.contains("id"))
                    uInf.id = std::move(jsonBody.at("id").as_string());
                if (jsonBody.contains("name"))
                    uInf.name = std::move(jsonBody.at("name").as_string());
                if (jsonBody.contains("email"))
                    uInf.email = std::move(jsonBody.at("email").as_string());
                if (jsonBody.contains("picture"))
                    uInf.photoUrl = std::move(jsonBody.at("picture").as_string());
                handler(uInf);
            } catch (const std::exception& e) {
                ReplyTypes::Types::Error obj;
                obj.errorMessage = std::string("JSON parsing error: ") + e.what();
                handler(obj);
                co_return;
            }
        },
        asio::detached);
}

http::request<http::empty_body> ClassroomManager::requestHandler(DTOCreateRequest&& dto) {
    if (authenticationManager_->EmptyAccessToken()) {
        http::request<http::empty_body> invalid;
        invalid.method(http::verb::unknown);
        return invalid;
    }

    auto result = std::async(std::launch::async, [this] {
        for (int i = 0; i < 20 && !authenticationManager_->EmptyAccessToken(); ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        return authenticationManager_->getToken();
    });

    http::request<http::empty_body> request;

    request.set(http::field::authorization, std::format("Bearer {}", result.get().data()));
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.method(http::verb::get);

    std::visit(
        util::match {
          [&request]([[maybe_unused]] DTOCourseList) {
              request.target("/v1/courses");
              request.set(http::field::accept, "application/json");
              request.set(http::field::host, GOOGLE_CLASSROOM_HOST);
          },
          [&request](DTOCourseWorksList courseWorksList) {
              request.target(std::format("/v1/courses/{}/courseWork", courseWorksList.courseId));
              request.set(http::field::accept, "application/json");
              request.set(http::field::host, GOOGLE_CLASSROOM_HOST);
          },
          [&request](DTOStudentsList studentsList) {
              request.target(std::format("/v1/courses/{}/students", studentsList.courseId));
              request.set(http::field::accept, "application/json");
              request.set(http::field::host, GOOGLE_CLASSROOM_HOST);
          },
          [&request](DTOStudentWorksDownload downloadWork) {
              request.target(std::format("/drive/v3/files/{}?alt=media", downloadWork.fileId));
              request.set(http::field::host, GOOGLE_HOST);
          },
          [&request]([[maybe_unused]] DTOUserInfo) {
              request.target("/oauth2/v1/userinfo?alt=json");
              request.set(http::field::accept, "application/json");
              request.set(http::field::host, GOOGLE_HOST);
          },
          [&request](DTOStudentWorksData studentWork) {
              request.target(std::format(
                  "/v1/courses/{}/courseWork/{}/studentSubmissions", studentWork.courseId, studentWork.courseWorkId));
              request.set(http::field::accept, "application/json");
              request.set(http::field::host, GOOGLE_CLASSROOM_HOST);
          } },
        dto);
    request.prepare_payload();
    return request;
}
}   // namespace Network
