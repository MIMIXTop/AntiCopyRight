#pragma once

#include <QObject>
#include <functional>

namespace KeyChainTokens {
    void saveRefreshToken(const QString& token, QObject* parent);
    void loadRefreshToken(std::function<void(QString)> callback, QObject* parent);
    void clearRefreshToken(QObject* parent);
}
