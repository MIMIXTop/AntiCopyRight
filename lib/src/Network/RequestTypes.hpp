#pragma once

#include <variant>
#include <QJsonArray>
#include <QFile>

#include "quazip.h"

namespace RequestTypes {
    struct Course {
        QJsonArray course;
    };

    struct CourseWorks {
        QJsonArray courseWorks;
    };

    struct DownloadStudentWork {
        QuaZip file;
    };

    struct StudentWorks {
        QJsonArray studentWorks;
    };

    using Request = std::variant<Course, CourseWorks, DownloadStudentWork, StudentWorks>;
}
