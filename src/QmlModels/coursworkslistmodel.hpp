#pragma once

#include <Network/NetworkManager.hpp>

#include <QAbstractListModel>
#include <QObject>

#include <vector>

class CoursWorksListModel : public QAbstractListModel {
  Q_OBJECT
 public:
  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    DescriptionRole,
    CourseIdRole,
    WorkIdRole
  };
  Q_ENUM(Roles)

  explicit CoursWorksListModel(QObject* parent = nullptr);

  QVariant data(const QModelIndex& index, int role) const override;
  int rowCount(const QModelIndex& paren = QModelIndex()) const override;
  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void updateWorks(const QString& courseId);

 private:
  struct Work {
    int id;
    QString name;
    QString description;
    QString courseId;
    QString workId;
  };

  void onReply(ReplyTypes::Reply reply);

  std::vector<Work> m_works;
  QString m_currentCourseId;
  Network::NetworkManager* nm = Network::NetworkManager::GetInstance();
};
