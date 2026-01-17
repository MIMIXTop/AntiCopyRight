#include "CourseModel.hpp"
#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qhashfunctions.h>
#include <qstringview.h>
#include <qtpreprocessorsupport.h>
#include <vector>

int CourseModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return courses.size();
}
QVariant CourseModel::data(const QModelIndex& index, int role) const {
    if (index.row() < 0 || index.row() >= courses.size())
        return {};

    const auto& course = courses[index.row()];
    switch (role) {
        case CourseNameRole:
            return course.courseName;
        case CourseIdRole:
            return course.courseId;
    }
    return {};
}
QHash<int, QByteArray> CourseModel::roleNames() const {
    QHash<int, QByteArray> rolse;
    rolse[CourseNameRole] = "name";
    rolse[CourseIdRole] = "id";
    return rolse;
}

void CourseModel::updateCourseInfo(std::vector<Course>&& newCourseInfo) {
    beginRemoveRows(QModelIndex(), 0, rowCount());
    courses.clear();
    endRemoveRows();
    beginInsertRows(QModelIndex(), 0, newCourseInfo.size());
    courses = std::move(newCourseInfo);
    endInsertRows();
}