#pragma once

#include <algorithm>
#include <boost/json/object.hpp>
#include <string>
#include <utility>
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
        Course(std::string&& id, std::string&& name) : courseId(std::move(id)), courseName(std::move(name)) {}
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

        CourseWork(std::string&& id, std::string&& courseId, std::string&& title, std::string&& description)
          : id(std::move(id))
          , courseId(std::move(courseId))
          , title(std::move(title))
          , description(std::move(description)) {}
    };
    std::vector<CourseWork> courseWorkList;
};
struct DownloadStudentWork {
    std::vector<uint8_t> courseWork;
    std::string fileName;

    DownloadStudentWork() = default;
    DownloadStudentWork(std::vector<uint8_t>&& work, std::string&& name)
      : courseWork(std::move(work)), fileName(std::move(name)) {}
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

            File(const std::string& fileId, const std::string& title, const std::string& sourseUrl,
                 const std::string& thumbnailUrl)
              : fileId(fileId), title(title), sourseUrl(sourseUrl), thumbnailUrl(thumbnailUrl) {}

            File(std::string&& fileId, std::string&& title, std::string&& sourseUrl, std::string&& thumbnailUrl)
              : fileId(std::move(fileId))
              , title(std::move(title))
              , sourseUrl(std::move(sourseUrl))
              , thumbnailUrl(std::move(thumbnailUrl)) {}
        };

        std::vector<File> files;

        StudentWork() = default;
        StudentWork(
            const std::string& id, const std::string& userId, const std::string& courseId,
            const std::string& courseWorkId)
          : id(id), userId(userId), courseId(courseId), courseWorkId(courseWorkId) {}

        StudentWork(std::string&& id, std::string&& userId, std::string&& courseId, std::string&& courseWorkId)
          : id(std::move(id))
          , userId(std::move(userId))
          , courseId(std::move(courseId))
          , courseWorkId(std::move(courseWorkId)) {}
    };
    std::vector<StudentWork> studentWorkList;
};

struct StudentList {
    std::string courseId;
    struct Student {
        std::string userId;
        std::string fullName;
        std::string email;
        std::string photoUrl;

        Student(const std::string& id, const std::string& fullName, const std::string& email, const std::string& photo)
          : userId(id), fullName(fullName), email(email), photoUrl(photo) {}

        Student(std::string&& id, std::string&& fullName, std::string&& email, std::string&& photo)
          : userId(std::move(id)), fullName(std::move(fullName)), email(std::move(email)), photoUrl(std::move(photo)) {}
    };
    std::vector<Student> studentsList;
};
struct Error {
    std::string errorMessage;

    Error() = default;
    Error(std::string&& message) : errorMessage(std::move(message)) {}
};

struct UserInfo {
    std::string id;
    std::string name;
    std::string photoUrl;
    std::string email;

    UserInfo() = default;
    UserInfo(const std::string& id, const std::string& name, const std::string& photoUrl, const std::string& email)
      : id(id), name(name), photoUrl(photoUrl), email(email) {}
    UserInfo(std::string&& id, std::string&& name, std::string&& photoUrl, std::string&& email)
      : id(std::move(id)), name(std::move(name)), photoUrl(std::move(photoUrl)), email(std::move(email)) {}
};
}   // namespace Types

using Reply = std::variant<
    Types::Courses, Types::CourseWorks, Types::StudentList, Types::StudentWorks, Types::DownloadStudentWork,
    Types::Error, Types::UserInfo>;
}   // namespace ReplyTypes
