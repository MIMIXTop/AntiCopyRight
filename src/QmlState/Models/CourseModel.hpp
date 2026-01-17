#pragma once

#include <QAbstractListModel>
#include <vector>

class CourseModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum CourseRoles { CourseNameRole = Qt::UserRole + 1, CourseIdRole };
    struct Course {
        QString courseName;
        QString courseId;
    };

    CourseModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void updateCourseInfo(std::vector<Course>&& newCourses);

private:
    std::vector<Course> courses;
};
