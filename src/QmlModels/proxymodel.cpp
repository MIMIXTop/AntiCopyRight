#include "proxymodel.hpp"

#include "coursworkslistmodel.hpp"

ProxyModel::ProxyModel(QObject *parent) : QSortFilterProxyModel{parent} {}

int ProxyModel::selectedCourseId() const
{
    return m_coursId;
}

void ProxyModel::setSelectedCourseId(int id)
{
    if (m_coursId == id) return;

    m_coursId = id;
    emit selectedCourseIdChanged();
    invalidateFilter();
}

bool ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const auto index = sourceModel()->index(sourceRow, 0, sourceParent);
    const int courseId = sourceModel()->data(index, CoursWorksListModel::CourseIdRole).toInt();
    return m_coursId == 0 ? true : (courseId == m_coursId);
}
