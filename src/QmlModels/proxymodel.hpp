#pragma once

#include <QObject>
#include <QSortFilterProxyModel>

class ProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int selectedCourseId READ selectedCourseId WRITE setSelectedCourseId NOTIFY selectedCourseIdChanged FINAL)
public:
    explicit ProxyModel(QObject *parent = nullptr);
    int selectedCourseId() const;
    void setSelectedCourseId(int id);

signals:
    void selectedCourseIdChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
private:
    int m_coursId{0};
};
