#pragma once

#include <variant>

#include <boost/beast/http/vector_body.hpp>

#include <boost/json/array.hpp>

namespace ReplyTypes {
    namespace BoostTypes {
        struct Course {
            boost::json::array course;
        };
        struct CourseWorks {
            boost::json::array courseWorks;
        };
        struct DownloadStudentWork {
            boost::beast::http::vector_body<uint8_t> courseWork;
        };
        struct StudentWorks {
            boost::json::array studentWorks;
        };
        struct StudentList {
            boost::json::array studentsList;
        };
    }

    using BoostReply = std::variant<BoostTypes::Course ,BoostTypes::CourseWorks ,BoostTypes::StudentList ,BoostTypes::StudentWorks ,BoostTypes::DownloadStudentWork>;
}



