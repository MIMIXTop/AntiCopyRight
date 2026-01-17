#pragma once

#include <qqml.h>
#include <QDebug>
#include "Network/ClassroomManager.hpp"
#include <QObject>
#include <utility>
#include "Models/CourseModel.hpp"
#include "Network/ReplyType.hpp"

class GlobalState : public QObject {
    Q_OBJECT

public:
    Q_PROPERTY(QString logo MEMBER userLogo NOTIFY UpdateLogo)
    Q_INVOKABLE void authUser();
    GlobalState() {
        // Инициализируем manager синхронно, чтобы избежать сегментации
        manager = std::make_unique<Network::ClassroomManager>();
        connect(this, &GlobalState::UpdateUserInfo, &GlobalState::updateUserData);
        if (!manager->emptyToken()) {
            initData();
        }
    }

signals:
    void UpdateSourse(QList<QString> list);
    void UpdateUserInfo(QString userName, QString UserLogo);
    void UpdateLogo();

private:
    void RequestHandler(ReplyTypes::Reply reply);
    void initData();

    void updateUserData(const QString& userName, const QString& userLogo) {
        this->userLogo = std::move(userLogo);
        qInfo() << "UserLogo: " << this->userLogo;
        emit UpdateLogo();
    }

    std::unique_ptr<Network::ClassroomManager> manager;
    QString userLogo;
    std::jthread networkThread;

    QList<QString> coursesName;
};