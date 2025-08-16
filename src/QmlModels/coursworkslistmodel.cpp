#include "coursworkslistmodel.hpp"

CoursWorksListModel::CoursWorksListModel(QJsonArray data, QObject *parent)
    : QAbstractListModel{parent}
{
    m_works.reserve(data.size());
    for (int i = 0;i < data.size(); ++i){
        m_works.push_back({
                            .id = i,
                            .name = data.at(i)["title"].toString(),
                            .description = data.at(i)["description"].toString(),
                            .courseId = data.at(i)["courseId"].toString(),
                            .workId = data.at(i)["id"].toString()
        });
    }
}

QVariant CoursWorksListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole){
        return {};
    }

    const auto work = m_works[index.row()];
    switch (role) {
    case IdRole: return work.id;
    case NameRole: return work.name;
    case DescriptionRole: return work.description;
    case CourseIdRole: return work.courseId;
    case WorkIdRole: return work.workId;
    }
    return {};
}

int CoursWorksListModel::rowCount(const QModelIndex &) const
{
    return m_works.size();
}

QHash<int, QByteArray> CoursWorksListModel::roleNames() const
{
    return {{IdRole, "id"}, {NameRole, "name"}, {DescriptionRole, "description"}, {CourseIdRole, "coursId"}, {WorkIdRole, "workId"}};
}
