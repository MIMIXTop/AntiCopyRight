#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "Network/AuthenticationManager.hpp"
#include "Network/ReplyType.hpp"
#include "Network/RequestDTO.hpp"
#include "Network/Session.hpp"

using HandlerFunction = std::function<void(ReplyTypes::Reply reply)>;

namespace Network {
class ClassroomManager : public std::enable_shared_from_this<ClassroomManager> {
public:
    ClassroomManager();
    ClassroomManager(
        std::unique_ptr<Session> ClassSess, std::unique_ptr<Session> GoogleSess,
        std::unique_ptr<AuthenticationManager> AuthMan);

    virtual ~ClassroomManager();

    void getCourses(HandlerFunction func);
    void getListCoursesWorks(const std::string& courseId, HandlerFunction func);
    void getStudentsList(const std::string& courseId, HandlerFunction func);
    void getStudentsWorks(const std::string& courseId, const std::string& courseWorkId, HandlerFunction func);
    void downloadStudentWork(const std::string& fileName, const std::string& fileId, HandlerFunction func);
    void getUserInfo(HandlerFunction func);

private:
    enum class RequestType { COURSES_LIST, COURSES_WORKS_LIST, STUDENTS_LIST, DOWNLOAD_STUDENT_WORK };
    boost::beast::http::request<boost::beast::http::empty_body> requestHandler(DTOCreateRequest&& dto);

    std::unique_ptr<Session> classroomSession_;
    std::unique_ptr<Session> googleSession_;
    std::unique_ptr<AuthenticationManager> authenticationManager_;

    boost::asio::io_context ioContext_;
    std::jthread networkThread;
};
}   // namespace Network
