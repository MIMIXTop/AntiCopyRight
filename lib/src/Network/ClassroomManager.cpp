#include "ClassroomManager.hpp"

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
#include <format>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include <Network/ReplyType.hpp>
#include <Network/RequestDTO.hpp>

#include "Util/util.hpp"

namespace {
#define GOOGLE_CLASSROOM_HOST "classroom.googleapis.com"
#define GOOGLE_HOST           "googleapis.com"

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
}   // namespace

namespace Network {
ClassroomManager::ClassroomManager() {
    networkThread = std::jthread([this] { ioContext_.run(); });

    classroomSession_ = std::make_unique<Session>(ioContext_);
    asio::co_spawn(ioContext_, classroomSession_->connectToSender(GOOGLE_CLASSROOM_HOST), asio::detached);

    googleSession_ = std::make_unique<Session>(ioContext_);
    asio::co_spawn(ioContext_, googleSession_->connectToSender(GOOGLE_HOST), asio::detached);

    authenticationManager_ = std::make_unique<AuthenticationManager>();
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
}

ClassroomManager::~ClassroomManager() { ioContext_.stop(); }

void ClassroomManager::getCourses(HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func)] -> asio::awaitable<void> {
            http::response<http::dynamic_body> res =
                co_await classroomSession_->sendRequest(requestHandler(DTOCourseList {}));
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.error = jsonBody;
                handler(obj);
                co_return;
            }
            ReplyTypes::Types::Courses obj;
            auto tempCourseList = jsonBody.at("courses").as_array();
            for (const auto& course : tempCourseList) {
                auto tempObj = course.as_object();
                obj.courseList.emplace_back(
                    tempObj.at("id").as_string().c_str(), tempObj.at("name").as_string().c_str());
            }
            handler(obj);
        },
        asio::detached);
}

void ClassroomManager::getListCoursesWorks(const std::string& courseId, HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func), courseId] -> asio::awaitable<void> {
            http::response<http::dynamic_body> res =
                co_await classroomSession_->sendRequest(requestHandler(DTOCourseWorksList { courseId }));
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.error = jsonBody;
                handler(obj);
                co_return;
            }

            ReplyTypes::Types::CourseWorks obj;
            auto tempObjectList = jsonBody.at("courseWork").as_array();
            for (auto&& courseWork : tempObjectList) {
                auto tempObj = courseWork.as_object();
                obj.courseWorkList.emplace_back(
                    tempObj.at("id").as_string().c_str(), tempObj.at("courseId").as_string().c_str(),
                    tempObj.at("title").as_string().c_str(), tempObj.at("description").as_string().c_str());
            }
            handler(obj);
        },
        asio::detached);
}

void ClassroomManager::getStudentWorks(
    const std::string& courseId, const std::string& courseWorkId, HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func), &courseId, &courseWorkId]() -> asio::awaitable<void> {
            http::response<http::dynamic_body> res = co_await classroomSession_->sendRequest(
                requestHandler(DTOStudentWorksData { .courseId = courseId, .courseWorkId = courseWorkId }));
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.error = jsonBody;
                handler(obj);
                co_return;
            }

            ReplyTypes::Types::StudentWorks obj;
            for (auto&& studentSubmission : jsonBody.at("studentSubmissions").as_array()) {
                std::string courseId = studentSubmission.as_object().at("courseId").as_string().c_str();
                std::string courseWorkId = studentSubmission.as_object().at("courseWorkId").as_string().c_str();
                std::string userId = studentSubmission.as_object().at("userId").as_string().c_str();
                std::string id = studentSubmission.as_object().at("id").as_string().c_str();
                ReplyTypes::Types::StudentWorks::StudentWork temp;
                temp.id = std::move(id);
                temp.courseWorkId = std::move(courseWorkId);
                temp.courseId = std::move(courseId);
                temp.userId = std::move(userId);

                for (
                    auto&& files :
                    studentSubmission.as_object().at("assignmentSubmission").as_object().at("attachments").as_array()) {
                    auto driveFiles = files.as_object().at("driveFile").as_object();
                    temp.files.emplace_back(
                        driveFiles.at("id").as_string().c_str(), driveFiles.at("title").as_string().c_str(),
                        driveFiles.at("alternateLink").as_string().c_str(),
                        driveFiles.at("thumbnailUrl").as_string().c_str());
                }
                obj.studentWorkList.push_back(std::move(temp));
            }
        },
        asio::detached);
}

void ClassroomManager::getStudentsList(const std::string& courseId, HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func), courseId] -> asio::awaitable<void> {
            http::response<http::dynamic_body> res =
                co_await classroomSession_->sendRequest(requestHandler(DTOStudentsList { courseId }));
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.error = jsonBody;
                handler(obj);
                co_return;
            }

            ReplyTypes::Types::StudentList obj;
            obj.studentsList = jsonBody.at("students").as_array();
            handler(obj);
        },
        asio::detached);
}

void ClassroomManager::downloadStudentWork(
    const std::string& fileName, const std::string& fileId, HandlerFunction func) {
    asio::co_spawn(
        ioContext_,
        [this, handler = std::move(func), fileName, fileId] -> asio::awaitable<void> {
            http::response<http::dynamic_body> res =
                co_await googleSession_->sendRequest(requestHandler(DTOStudentWorksDownload { fileId }));

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                auto body = beast::buffers_to_string(res.body().cdata());
                auto jsonBody = json::parse(body).as_object();
                obj.error = jsonBody;
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
            http::response<http::dynamic_body> res =
                co_await googleSession_->sendRequest(requestHandler(DTOUserInfo {}));
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();

            if (res.result() != http::status::ok) {
                ReplyTypes::Types::Error obj;
                obj.error = jsonBody;
                handler(obj);
                co_return;
            }
            ReplyTypes::Types::UserInfo uInf;
            uInf.userInfoDate = jsonBody;
            handler(uInf);
        },
        asio::detached);
}

http::request<http::empty_body> ClassroomManager::requestHandler(DTOCreateRequest&& dto) {
    if (authenticationManager_->EmptyAccessToken()) {
        http::request<http::empty_body> invalid;
        invalid.method(http::verb::unknown);
        return invalid;
    }

    http::request<http::empty_body> request;
    request.set(http::field::authorization, std::format("Bearer {}", authenticationManager_->getToken()));
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
