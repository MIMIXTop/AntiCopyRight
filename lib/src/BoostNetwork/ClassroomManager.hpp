#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <memory>
#include <BoostNetwork/AuthenticationManager.hpp>
#include <BoostNetwork/Session.hpp>
#include <string>
#include <thread>
#include <functional>

#include "ReplyType.hpp"
#include "RequestDTO.hpp"

using HandlerFunction = std::function<void(ReplyTypes::BoostReply reply)>;

namespace Network {
    class ClassroomManager : public std::enable_shared_from_this<ClassroomManager> {
    public:
        ClassroomManager();
        ClassroomManager(std::unique_ptr<Session> ClassSess, std::unique_ptr<Session> DriveSess, std::unique_ptr<AuthenticationManager> authMan);

        virtual ~ClassroomManager();

        void getCourses(HandlerFunction func);
        void getListCoursesWorks(HandlerFunction func, const std::string& courseId);
        void getStudentsList(HandlerFunction func, const std::string& courseId);
        void downloadStudentWork(HandlerFunction func, const std::string& fileName, const std::string& fileId);

    private:
        enum class RequestType {COURSES_LIST, COURSES_WORKS_LIST, STUDENTS_LIST, DOWNLOAD_STUDENT_WORK};
        boost::beast::http::request<boost::beast::http::empty_body> requestHandler(DTOCreateRequest&& dto);

        std::unique_ptr<Session> classroomSession_;
        std::unique_ptr<Session> driveSession_;
        std::unique_ptr<AuthenticationManager> authenticationManager_;

        boost::asio::io_context ioContext_;
        std::jthread networkThread;
    };
} // Network
