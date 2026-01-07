#include "wordssimilaritymodel.hpp"

WordsSimilarityModel::WordsSimilarityModel(QObject* parent)
    : QAbstractTableModel{parent} {}

int WordsSimilarityModel::rowCount(const QModelIndex& parent) const {
  return m_worksSimilarity.size();
}

int WordsSimilarityModel::columnCount(const QModelIndex& parent) const {
  return m_worksSimilarity.size();
}

QVariant WordsSimilarityModel::data(const QModelIndex& index, int role) const {}

QVariant WordsSimilarityModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {}

QHash<int, QByteArray> WordsSimilarityModel::roleNames() const {}
