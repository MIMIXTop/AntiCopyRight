#include "Network/ClassroomManager.hpp"
#include <QObject>

class GlobalState : public QObject {
    Q_OBJECT

public:
    Q_INVOKABLE void updateCource();

    GlobalState() { manager = new Network::ClassroomManager(); }

signals:
    void UpdateSourse(QList<QString> list);

private:
    Network::ClassroomManager* manager;

    QList<QString> coursesName;
};