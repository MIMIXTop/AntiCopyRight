#include "courselistmodel.hpp"

#include <Network/ReplyTypes.hpp>
#include <Util/util.hpp>
#include <QNetworkReply>

#include <variant>

CourseListModel::CourseListModel( QObject *parent)
    : QAbstractListModel{parent}
{
    manager->getListCourses();

    connect(manager, &Network::NetworkManager::responseToRequest, [this](ReplyTypes::Reply reply){
        beginResetModel();
        std::visit(util::match{
            [this](const ReplyTypes::Type::Course &course) {
                    const auto data = course.course;
                    m_courses.clear();
                    beginInsertRows(QModelIndex(), 0, data.size() - 1);
                    for (int i = 0; i < data.size(); ++i) {
                        Course cc = { data.at(i)["name"].toString(),
                            data.at(i)["id"].toString(),
                            i };
                        m_courses.push_back(std::move(cc));
                    }
                    endInsertRows();
            } ,
            []([[maybe_unused]] auto other) {
                qWarning() << "Unexpected reply type:" << typeid(other).name();
            }
        },reply);
        endResetModel();
    });

    connect(manager, &Network::NetworkManager::requestFailed,[this](QString error){
        qInfo() << error;
    });
}

QVariant CourseListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto cours = m_courses[index.row()];

    switch (role) {
        case IdRole: return cours.id;
        case CoursIdRole: return cours.CoursId;
        case NameRole: return cours.name;
        default: return {};
    }
}

int CourseListModel::rowCount(const QModelIndex &) const
{
    return m_courses.size();
}

QHash<int, QByteArray> CourseListModel::roleNames() const
{
    return {{IdRole,"id"}, {NameRole, "nameCourse"}, {CoursIdRole, "courseId"}};
}
