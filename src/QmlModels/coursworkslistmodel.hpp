#pragma once

#include <Network/NetworkManager.hpp>

#include <QObject>
#include <QAbstractListModel>

#include <vector>

class CoursWorksListModel final : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {IdRole = Qt::UserRole + 1, NameRole, DescriptionRole, CourseIdRole, WorkIdRole};
    Q_ENUM(Roles)

    explicit CoursWorksListModel(QJsonArray data, QObject *parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex&) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Work {
        int id;
        QString name;
        QString description;
        QString courseId;
        QString workId;
    };

    std::vector<Work> m_works;
};
