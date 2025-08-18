#include "proxymodel.hpp"

#include "coursworkslistmodel.hpp"

ProxyModel::ProxyModel(QObject *parent) : QSortFilterProxyModel{parent} {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);
}

QString ProxyModel::selectedCourseId() const
{  
    return m_coursId;
}

void ProxyModel::setSelectedCourseId(const QString& id)
{
    if (m_coursId == id) return;
    m_coursId = id;
    emit selectedCourseIdChanged();
    invalidateFilter();
}

bool ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if(!sourceModel()) return true;
    const auto index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!index.isValid()) return false;

    const QString courseId = sourceModel()->data(index, CoursWorksListModel::CourseIdRole).toString();
    return m_coursId.isEmpty() ? true : (courseId == m_coursId);
}
