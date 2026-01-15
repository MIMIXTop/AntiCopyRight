#include <QDebug>
#include "Network/ClassroomManager.hpp"
#include <QObject>
#include <utility>

class GlobalState : public QObject {
    Q_OBJECT

public:
    Q_PROPERTY(QString logo MEMBER userLogo NOTIFY UpdateLogo)
    Q_INVOKABLE void authUser();
    GlobalState() {
        networkThread = std::jthread([this] { manager = std::make_unique<Network::ClassroomManager>(); });
        connect(this, &GlobalState::UpdateUserInfo, &GlobalState::updateUserData);
    }

signals:
    void UpdateSourse(QList<QString> list);
    void UpdateUserInfo(QString userName, QString UserLogo);
    void UpdateLogo();

private:
    void updateUserData(const QString& userName, const QString& userLogo) {
        userInfo.name = std::move(userName);
        this->userLogo = std::move(userLogo);
        qInfo() << "UserLogo: " << this->userLogo;
        emit UpdateLogo();
    }

    std::unique_ptr<Network::ClassroomManager> manager;
    struct {
        QString name;
        QString logo;
    } userInfo;
    QString userLogo;
    std::jthread networkThread;

    QList<QString> coursesName;
};