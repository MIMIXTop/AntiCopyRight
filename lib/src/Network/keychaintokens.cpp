#include "keychaintokens.hpp"

#include <qtkeychain/keychain.h>
#include <QObject>
#include <QDebug>

static const QString kService{"AntiCopyRight"};
static const QString kKey{"google.refresh_token"};

void KeyChainTokens::saveRefreshToken(const QString &token, QObject *parent) {
    auto* job = new QKeychain::WritePasswordJob(kService, parent);
    job->setKey(kKey);
    job->setTextData(token);
    QObject::connect(job, &QKeychain::Job::finished,[job](){
        if (job->error()) qWarning() << "Keychain save failed:" << job->errorString();
        job->deleteLater();
    });
    job->start();
}

void KeyChainTokens::loadRefreshToken(std::function<void (QString)> callback, QObject *parent) {
    auto* job = new QKeychain::ReadPasswordJob(kService, parent);
    job->setKey(kKey);
    QObject::connect(job, &QKeychain::Job::finished, [job, callback = std::move(callback)](){
        QString token;
        if(job->error() == QKeychain::NoError)
            token = job->textData();
        else if (job->error() != QKeychain::EntryNotFound)
            qWarning() << "Keychain read failed: " << job->errorString();
        callback(token);
        job->deleteLater();
    });
    job->start();
}

void KeyChainTokens::clearRefreshToken(QObject *parent) {
    auto *job = new QKeychain::DeletePasswordJob(kService, parent);
    QObject::connect(job, &QKeychain::Job::finished, [job](){ job->deleteLater(); });
    job->start();
}
