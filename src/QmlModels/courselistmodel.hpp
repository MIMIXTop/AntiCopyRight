#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <Network/NetworkManager.hpp>

#include <vector>

class CourseListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles{IdRole = Qt::UserRole + 1, NameRole, CoursIdRole};
    Q_ENUM(Roles)

    explicit CourseListModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex&) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Course {
        QString name;
        QString CoursId;
        int id;
    };

    std::vector<Course> m_courses;
    Network::NetworkManager *manager = Network::NetworkManager::GetInstance();
};
