#pragma once

#include <boost/json/object.hpp>
#include <string>
#include <variant>
#include <vector>

#include <boost/beast/http/vector_body.hpp>

#include <boost/json/array.hpp>

namespace ReplyTypes {
namespace Types {
struct Courses {
    struct Course {
        std::string courseName;
        std::string courseId;

        Course(const std::string& id, const std::string& name) : courseId(id), courseName(name) {}
    };
    std::vector<Course> courseList;
};
struct CourseWorks {
    struct CourseWork {
        std::string id;
        std::string courseId;
        std::string title;
        std::string description;

        CourseWork(
            const std::string& id, const std::string& courseId, const std::string& title,
            const std::string& description)
          : id(id), courseId(courseId), title(title), description(description) {}
    };
    std::vector<CourseWork> courseWorkList;
};
struct DownloadStudentWork {
    std::vector<uint8_t> courseWork;
    std::string fileName;
};
struct StudentWorks {
    struct StudentWork {
        std::string id;
        std::string userId;
        std::string courseId;
        std::string courseWorkId;

        struct File {
            std::string fileId;
            std::string title;
            std::string sourseUrl;
            std::string thumbnailUrl;

            File(const std::string fileId, const std::string& title, const std::string& sourseUrl,
                 const std::string& thumbnailUrl)
              : fileId(fileId), title(title), sourseUrl(sourseUrl), thumbnailUrl(thumbnailUrl) {}
        };

        std::vector<File> files;
    };
    std::vector<StudentWork> studentWorkList;
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
}   // namespace Types

using Reply = std::variant<
    Types::Courses, Types::CourseWorks, Types::StudentList, Types::StudentWorks, Types::DownloadStudentWork,
    Types::Error, Types::UserInfo>;
}   // namespace ReplyTypes
