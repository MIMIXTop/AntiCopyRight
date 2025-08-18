#pragma once

#include <QObject>
#include <QSortFilterProxyModel>

class ProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString selectedCourseId READ selectedCourseId WRITE setSelectedCourseId NOTIFY selectedCourseIdChanged FINAL)
public:
    explicit ProxyModel(QObject *parent = nullptr);
    QString selectedCourseId() const;
    void setSelectedCourseId(const QString& id);

signals:
    void selectedCourseIdChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
private:
    QString m_coursId;
};
