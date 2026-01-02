#pragma once

#include <memory>
#include <BoostNetwork/AuthenticationManager.hpp>
#include <BoostNetwork/Session.hpp>

#include "ReplyType.hpp"
#include "RequestDTO.hpp"

namespace  asio = boost::asio;

namespace Network {
    class ClassroomManager : public std::enable_shared_from_this<ClassroomManager> {
    public:
        ClassroomManager();

        ~ClassroomManager() = default;

        ReplyTypes::BoostReply getCourses();
        ReplyTypes::BoostReply getListCoursesWorks(const std::string& courseId);
        ReplyTypes::BoostReply getStudentsList(const std::string& courseId);
        ReplyTypes::BoostReply downloadStudentWork(const std::string& fileName, const std::string& fileUrl);

    private:
        enum class RequestType {COURSES_LIST, COURSES_WORKS_LIST, STUDENTS_LIST, DOWNLOAD_STUDENT_WORK};
        boost::beast::http::request<boost::beast::http::empty_body> requestHandler(DTOCreateRequest dto);

        std::unique_ptr<Session> classroomSession_;
        std::unique_ptr<Session> driveSession_;
        std::unique_ptr<AuthenticationManager> authenticationManager_;

        boost::asio::io_context ioContext_;
    };
} // Network
