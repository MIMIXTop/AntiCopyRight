#pragma once

#include <boost/json/object.hpp>
#include <string>
#include <variant>
#include <vector>

#include <boost/beast/http/vector_body.hpp>

#include <boost/json/array.hpp>

namespace ReplyTypes {
namespace Types {
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

struct UserInfo {
  boost::json::object userInfoDate;
};
} // namespace Types

using Reply =
    std::variant<Types::Course, Types::CourseWorks,
                 Types::StudentList, Types::StudentWorks,
                 Types::DownloadStudentWork, Types::Error,
                 Types::UserInfo>;
} // namespace ReplyTypes
