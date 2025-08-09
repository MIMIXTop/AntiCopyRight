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
    }

    using Reply = std::variant<Type::Course, Type::CourseWorks, Type::DownloadStudentWork, Type::StudentWorks>;
}
