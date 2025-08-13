#include "NetworkManager.hpp"
#include "ReplyTypes.hpp"

#include <QFile>
#include <QDir>
#include <QBuffer>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <quazip/quazipnewinfo.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QOAuthHttpServerReplyHandler>
#include <QTimer>

namespace {
#ifdef WIN32
    #define CREDENTIALS_PATH "Util\\Network\\init.json"
#else
    #define CREDENTIALS_PATH "../Utils/Network/init.json"
#endif
}

namespace Network {
    NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
        google = new QOAuth2AuthorizationCodeFlow(this);
        manager = new QNetworkAccessManager(this);
        replyHandler = new QOAuthHttpServerReplyHandler(this);

        google->setReplyHandler(replyHandler);
        connect(google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);
        connect(google, &QOAuth2AuthorizationCodeFlow::granted, [this]() {
            this->connectedStatus = ConnectionState::Connected;
        });

        if (std::filesystem::exists(CREDENTIALS_PATH)) {
            qInfo() << "Credentials";
        } else {
            qInfo() << "No CREDENTIALS";
        }

        QFile file(CREDENTIALS_PATH);
        file.open(QIODevice::ReadOnly);

        QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        QJsonObject json = document.object()["installed"].toObject();

        google->setRequestedScopeTokens({
            "https://www.googleapis.com/auth/classroom.coursework.students",
            "https://www.googleapis.com/auth/classroom.coursework.students.readonly",
            "https://www.googleapis.com/auth/drive.readonly",
            "https://www.googleapis.com/auth/classroom.courses"
        });

        google->setClientIdentifier(json["client_id"].toString());
        google->setToken(json["client_secret"].toString());
        google->setAuthorizationUrl(QUrl(json["auth_uri"].toString()));
        google->setTokenUrl(QUrl(json["token_uri"].toString()));
    }


    void NetworkManager::authenticate() const {
        google->grant();
    }

    void NetworkManager::getListCourses() {
        enqueueWhenConnecting("getListCourses", 10000, [this]() {
           startCoursesRequest();
        });
    }

    void NetworkManager::getStudentsWorks(const QString& coursId, const QString& coursWorkId) {
        enqueueWhenConnecting("getStudentsWorks", 10000, [this, coursId, coursWorkId]() {
            startStudentsWorksRequest(coursId, coursWorkId);
        });
    }

    void NetworkManager::downloadStudentWork(const QString &fileName, const QString &fileUrl) {
        enqueueWhenConnecting("downloadStudentWork", 10000, [this, fileName, fileUrl]() {
           startDownloadStudentWorksRequest(fileName, fileUrl);
        });
    }

    void NetworkManager::getListCoursesWorks(const QJsonObject &course) {
        enqueueWhenConnecting("getListCoursesWorks", 10000, [this, course]() {
            startCourseWorksRequest(course);
        });
    }

    void NetworkManager::setConnectionState(ConnectionState state) {
        if (state == connectedStatus) return;
        connectedStatus = state;
        emit statusChanged(state);

        if (state == ConnectionState::Connecting) {
            drainPendingConnecting();
        } else if (state == ConnectionState::Disconnecting) {
            failAllPending("Not authenticated");
        }
    }

    void NetworkManager::enqueueWhenConnecting(const QString &name, quint64 timoutMs, std::function<void()> action) {
        if (connectedStatus == ConnectionState::Disconnecting) {
            emit requestFailed(name + ": Not authenticated");
            return;
        }
        if (connectedStatus == ConnectionState::Connecting) {
            action();
            return;
        }

        const quint64 id = NextId++;
        PendingAction pendingAction{ id, name, std::move(action), new QTimer(this) };
        pendingAction.timer->setSingleShot(true);
        pendingAction.timer->setInterval(timoutMs);

        connect(pendingAction.timer, &QTimer::timeout,this , [this, id]() {
            if (!queueActions.contains(id)) return;
            const QString nm = queueActions[id].name;
            queueActions.remove(id);
            emit requestFailed(nm + ": Timeout waiting for Connecting");
        });

        pendingAction.timer->start();
        queueActions.insert(id, pendingAction);
    }

    void NetworkManager::drainPendingConnecting() {
        if (connectedStatus == ConnectionState::Connecting) return;

        const auto ids = queueActions.keys();
        for (auto&& id : ids) {
            auto it = queueActions.find(id);
            if (it == queueActions.end()) continue;
            if (it->timer) it->timer->stop();
            auto action = it->action;
            queueActions.erase(it);
            if (action) action();
        }
    }

    void NetworkManager::failAllPending(const QString &reason) {
        const auto ids = queueActions.keys();
        for (auto&& id : ids) {
            auto it = queueActions.find(id);
            if (it == queueActions.end()) continue;
            if (it->timer) it->timer->stop();
            const QString nm = queueActions[id].name;
            queueActions.erase(it);
            emit requestFailed(nm + ": " + reason);
        }
    }

    void NetworkManager::startCoursesRequest() {
        QNetworkRequest request(QUrl("https://classroom.googleapis.com/v1/courses"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", "Bearer " + google->token().toLatin1());

        QNetworkReply *reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                emit requestFailed("Network error: " + reply->errorString());
                reply->deleteLater();
                return;
            }

            QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
            if (!document.isObject() || !document.object().contains("courses")) {
                emit requestFailed("Invalid JSON response!");
                reply->deleteLater();
                return;
            }

            emit responseToRequest(ReplyTypes::Type::Course(document.object()["courses"].toArray()));
            reply->deleteLater();
        });
    }

    void NetworkManager::startStudentsWorksRequest(const QString &courseId, const QString &courseWorkId) {
        QNetworkRequest request(QUrl("https://classroom.googleapis.com/v1/courses/" + courseId + "/courseWork/" + courseWorkId + "/studentSubmissions"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", "Bearer " + google->token().toLatin1());
        QNetworkReply *reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                emit requestFailed("Network error: " + reply->errorString());
                reply->deleteLater();
                return;
            }

            QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
            if (!document.isObject() || !document.object().contains("studentSubmissions")) {
                emit requestFailed("Invalid JSON response!");
                reply->deleteLater();
                return;
            }

            emit responseToRequest(ReplyTypes::Type::StudentWorks(document.object()["studentSubmissions"].toArray()));
        });
    }

    void NetworkManager::startCourseWorksRequest(const QJsonObject &course) {
        QNetworkRequest request(QUrl("https://classroom.googleapis.com/v1/courses/" + course["id"].toString() + "/courseWork"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        request.setRawHeader("Authorization", "Bearer " + google->token().toLatin1());

        QNetworkReply* reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                emit requestFailed("Network error: " + reply->errorString());
                reply->deleteLater();
                return;
            }

            QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
            if (!document.isObject() || !document.object().contains("courseWork")) {
                emit requestFailed("Invalid JSON response!");
                reply->deleteLater();
                return;
            }

            emit responseToRequest(ReplyTypes::Type::CourseWorks(document.object()["courseWork"].toArray()));
            reply->deleteLater();
        });
    }

    void NetworkManager::startDownloadStudentWorksRequest(const QString &fileName, const QString &fileId) {
        QNetworkRequest request(QUrl(QString("https://www.googleapis.com/drive/v3/files/%1?alt=media").arg(fileId)));

        request.setRawHeader("Authorization", "Bearer " + google->token().toLatin1());
        QNetworkReply *reply = manager->get(request);

        QuaZipFileInfo fileInfo(fileName);
        QBuffer *buffer = new QBuffer();
        buffer->open(QIODevice::WriteOnly);

        QuaZip zip(buffer);
        zip.open(QuaZip::mdCreate);

        QuaZipFile file(&zip);
        file.open(QIODevice::WriteOnly);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                emit requestFailed("Network error: " + reply->errorString());
                reply->deleteLater();
                return;
            }

            emit responseToRequest(ReplyTypes::Type::DownloadStudentWork(reply->readAll()));
        });
    }
} // Network
