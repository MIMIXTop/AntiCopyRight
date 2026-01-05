#include "ClassroomManager.hpp"

#include <algorithm>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>
#include <QObject>
#include <boost/asio/post.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/json/parse.hpp>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "BoostNetwork/ReplyType.hpp"
#include "BoostNetwork/RequestDTO.hpp"
#include "Util/util.hpp"

namespace {
#define GOOGLE_CLASSROOM_HOST "classroom.googleapis.com"
#define GOOGLE_HOST "googleapis.com"

    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;
}

namespace Network {
    ClassroomManager::ClassroomManager() {
        networkThread = std::jthread([this]{
            ioContext_.run();
        });

        classroomSession_ = std::make_unique<Session>(ioContext_);
        asio::co_spawn(ioContext_, classroomSession_->connectToSender(GOOGLE_CLASSROOM_HOST), asio::detached);


        driveSession_ = std::make_unique<Session>(ioContext_);
        asio::co_spawn(ioContext_, driveSession_->connectToSender(GOOGLE_HOST), asio::detached);

        authenticationManager_ = std::make_unique<AuthenticationManager>();
    }

    ClassroomManager::ClassroomManager(std::unique_ptr<Session> ClassSess, std::unique_ptr<Session> DriveSess, std::unique_ptr<AuthenticationManager> authMan) : 
    classroomSession_(std::move(ClassSess)), driveSession_(std::move(DriveSess)), authenticationManager_(std::move(authMan)) {
           networkThread = std::jthread([this]{
            auto work = asio::make_work_guard(ioContext_);
            ioContext_.run();
        });
 
    }


    ClassroomManager::~ClassroomManager() {
        ioContext_.stop();
    }

    void ClassroomManager::getCourses(HandlerFunction func) {
        asio::co_spawn(ioContext_, [this, handler = std::move(func)] -> asio::awaitable<void>{
            http::response<http::dynamic_body> res = co_await classroomSession_->sendRequest(requestHandler(DTOCourseList{}));
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();
            ReplyTypes::BoostTypes::Course obj;
            obj.course = jsonBody.at("courses").as_array();
            handler(obj);
        }, asio::detached);
    }

    void ClassroomManager::getListCoursesWorks(HandlerFunction func, const std::string &courseId) {
        asio::co_spawn(ioContext_, [this, handler = std::move(func), courseId] -> asio::awaitable<void>{
            http::response<http::dynamic_body> res = co_await classroomSession_->sendRequest(requestHandler(DTOCourseWorksList{courseId}));
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();
            ReplyTypes::BoostTypes::CourseWorks obj;
            obj.courseWorks = jsonBody.at("courseWork").as_array();
            handler(obj);
        }, asio::detached);
    }

    void ClassroomManager::getStudentsList(HandlerFunction func, const std::string &courseId) {
        asio::co_spawn(ioContext_, [this, handler = std::move(func), courseId] -> asio::awaitable<void>{
            http::response<http::dynamic_body> res = co_await classroomSession_->sendRequest(requestHandler(DTOStudentsList{courseId}));
            auto body = beast::buffers_to_string(res.body().cdata());
            auto jsonBody = json::parse(body).as_object();
            ReplyTypes::BoostTypes::CourseWorks obj;
            obj.courseWorks = jsonBody.at("students").as_array();
            handler(obj);
        }, asio::detached);
    }

    void ClassroomManager::downloadStudentWork(HandlerFunction func, const std::string &fileName, const std::string &fileId) {
        asio::co_spawn(ioContext_, [this, handler = std::move(func), fileName, fileId] -> asio::awaitable<void> {
            http::response<http::dynamic_body> res = co_await driveSession_->sendRequest(requestHandler(DTOStudentWorksDownload{fileId}));
            auto bodyBegin = asio::buffers_begin(res.body().cdata());
            auto bodyEnd = asio::buffers_end(res.body().cdata());
            ReplyTypes::BoostTypes::DownloadStudentWork obj;
            obj.courseWork = std::vector<uint8_t>{bodyBegin, bodyEnd};
            obj.fileName = fileName;
            handler(obj);
        }, asio::detached);
    }
    
    http::request<http::empty_body> ClassroomManager::requestHandler(DTOCreateRequest&& dto) {
        if (authenticationManager_->EmptyAccessToken()) {
            http::request<http::empty_body> invalid;
            invalid.method(http::verb::unknown);
            return invalid;
        }

        http::request<http::empty_body> request;
        request.set(http::field::authorization,
                    std::format("Bearer {}", authenticationManager_->getToken()));
        request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        request.method(http::verb::get);

        std::visit(util::match{
                       [&request]([[maybe_unused]] DTOCourseList courseList) {
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
                           request.target(std::format("/drive/v3/files/{}&alt=media", downloadWork.fileId));
                           request.set(http::field::host, GOOGLE_HOST);
                       }
                   }, dto);
        request.prepare_payload();           
        return request;
    }
} // Network
