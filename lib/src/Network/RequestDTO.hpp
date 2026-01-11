#pragma once

#include <string>
#include <variant>

struct DTOCourseList {};

struct DTOCourseWorksList {
    std::string courseId;
};

struct DTOStudentsList {
    std::string courseId;
};

struct DTOStudentWorksDownload {
    std::string fileId;
};

struct DTOStudentWorksData {
    std::string courseId;
    std::string courseWorkId;
};

struct DTOUserInfo {};

using DTOCreateRequest = std::variant<
    DTOCourseList, DTOCourseWorksList, DTOStudentsList, DTOStudentWorksDownload, DTOUserInfo, DTOStudentWorksData>;