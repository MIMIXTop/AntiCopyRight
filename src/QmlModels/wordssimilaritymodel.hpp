#pragma once

#include <QObject>
#include <QAbstractTableModel>
#include <Network/NetworkManager.hpp>
#include <qobject.h>
#include <qtmetamacros.h>

#pragma push_macro("slots")
#undef slots

#include <TextAnalyzer/Model/Doc2VecModel.hpp>

#pragma pop_macro("slots")

class WordsSimilarityModel final : public QAbstractTableModel
{
    enum Role{
        UserIdRole = Qt::UserRole + 1,
        DocumentNameRole,
        SimilarityRole
    };
    Q_OBJECT
public: 
    Q_ENUM(Role)
    explicit WordsSimilarityModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void calculateSimilarity();

private:
    struct documentItem
    {
        QString userId;
        QString documentName;
        torch::Tensor documentVector;
    };

    Network::NetworkManager *manager = Network::NetworkManager::GetInstance();
    Model::Doc2VecModel *model;
    QString m_crrentCourseId;
    std::vector<documentItem> m_worksSimilarity;
};
