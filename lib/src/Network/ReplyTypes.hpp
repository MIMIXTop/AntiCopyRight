#pragma once

#include <variant>
#include <QJsonArray>
#include <QByteArray>

namespace ReplyTypes {
    namespace Type {
        struct Course {
            QJsonArray course;
        };

        struct CourseWorks {
            QJsonArray courseWorks;
        };

        struct DownloadStudentWork {
            QByteArray courseWork;
        };

        struct StudentWorks {
            QJsonArray studentWorks;
        };

        struct StudentList
        {
            QJsonArray studentsList;
        };
    }

    using Reply = std::variant<Type::Course, Type::CourseWorks, Type::DownloadStudentWork, Type::StudentWorks, Type::StudentList>;
}
