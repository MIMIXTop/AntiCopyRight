#include "coursworkslistmodel.hpp"
#include <Network/ReplyTypes.hpp>
#include <Util/util.hpp>

#include <variant>

#include <QDebug>

CoursWorksListModel::CoursWorksListModel(QObject* parent)
    : QAbstractListModel{parent} {
  connect(nm, &Network::NetworkManager::responseToRequest, this,
          &CoursWorksListModel::onReply, Qt::QueuedConnection);
  connect(
      nm, &Network::NetworkManager::requestFailed, this,
      [](const QString err) { qWarning() << "Works request failed:" << err; });
}

QVariant CoursWorksListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
    return {};

  const auto work = m_works[index.row()];
  switch (role) {
  case IdRole:
    return work.id;
  case NameRole:
    return work.name;
  case DescriptionRole:
    return work.description;
  case CourseIdRole:
    return work.courseId;
  case WorkIdRole:
    return work.workId;
  }

  return {};
}

int CoursWorksListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid())
    return 0;
  return m_works.size();
}

QHash<int, QByteArray> CoursWorksListModel::roleNames() const {
  return {{IdRole, "id"},
          {NameRole, "name"},
          {DescriptionRole, "description"},
          {CourseIdRole, "courseId"},
          {WorkIdRole, "workId"}};
}

void CoursWorksListModel::updateWorks(const QString& courseId) {
  m_currentCourseId = courseId;
  nm->getListCoursesWorks(courseId);
}

void CoursWorksListModel::onReply(ReplyTypes::Reply reply) {
  beginResetModel();
  std::visit(util::match{[this](ReplyTypes::Type::CourseWorks courseWorks) {
                           const auto data = courseWorks.courseWorks;
                           m_works.clear();
                           m_works.reserve(data.size());
                           for (int i = 0; i < data.size(); ++i) {
                             Work work = {i, data.at(i)["title"].toString(),
                                          data.at(i)["description"].toString(),
                                          data.at(i)["courseId"].toString(),
                                          data.at(i)["id"].toString()};
                             m_works.push_back(work);
                           }
                         },
                         [](auto other) {
                           qWarning() << "Unexpected reply type:"
                                      << typeid(other).name();
                         }},
             reply);
  endResetModel();
}
