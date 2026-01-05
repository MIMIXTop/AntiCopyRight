#pragma once

#include <boost/json/object.hpp>
#include <string>
#include <variant>
#include <vector>

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
            std::vector<uint8_t> courseWork;
            std::string fileName;
        };
        struct StudentWorks {
            boost::json::array studentWorks;
        };
        struct StudentList {
            boost::json::array studentsList;
        };
        struct Error {
            boost::json::object error;
        };
    }
    
    using BoostReply = std::variant<BoostTypes::Course ,BoostTypes::CourseWorks ,BoostTypes::StudentList ,BoostTypes::StudentWorks ,BoostTypes::DownloadStudentWork, BoostTypes::Error>;
}



