#include "ClassroomManager.hpp"


#include <boost/asio.hpp>

#include "Util/util.hpp"

namespace {
#define GOOGLE_CLASSROOM_HOST "classroom.googleapis.com"
#define GOOGLE_HOST "googleapis.com"

    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace http = beast::http;
}

namespace Network {
    ClassroomManager::ClassroomManager() {
        classroomSession_ = std::make_unique<Session>(ioContext_);
        classroomSession_->connectToSender(GOOGLE_CLASSROOM_HOST);

        driveSession_ = std::make_unique<Session>(ioContext_);
        driveSession_->connectToSender(GOOGLE_HOST);

        authenticationManager_ = std::make_unique<AuthenticationManager>();
    }

    ReplyTypes::BoostReply ClassroomManager::getCourses() {
    }

    ReplyTypes::BoostReply ClassroomManager::getListCoursesWorks(const std::string &courseId) {
    }

    ReplyTypes::BoostReply ClassroomManager::getStudentsList(const std::string &courseId) {
    }

    ReplyTypes::BoostReply ClassroomManager::downloadStudentWork(const std::string &fileName,
                                                                 const std::string &fileUrl) {
    }

    http::request<http::empty_body> ClassroomManager::requestHandler(DTOCreateRequest dto) {
        if (authenticationManager_->EmptyAccessToken()) {
            http::request<http::empty_body> invalid;
            invalid.method(http::verb::unknown); // или просто бросить исключение
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
        return request;
    }
} // Network
