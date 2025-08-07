#include "NetworkManager.hpp"
#include "NetworkManager.hpp"
#include "NetworkManager.hpp"

#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QOAuthHttpServerReplyHandler>

namespace {
#ifdef WIN32
    #define CREDENTIALS_PATH "Util\\Network\\credentials.json"
#else
    #define CREDENTIALS_PATH "Util/Network/credentials.json"
#endif
}

namespace Network {
    NetworkManager::NetworkManager() : google(new QOAuth2AuthorizationCodeFlow()),
                                       manager(new QNetworkAccessManager),
                                       replyHandler(new QOAuthHttpServerReplyHandler) {
        google->setReplyHandler(replyHandler);
        connect(google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);
        connect(google, &QOAuth2AuthorizationCodeFlow::granted, [this]() {
            this->connectedStatus = true;
        });

        QFile file(CREDENTIALS_PATH);
        if (!file.open(QIODevice::ReadOnly)) {
            throw std::runtime_error("You need add credentials.json 'Google Cloud API'");
        }

        QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        QJsonObject json = document.object()["installed"].toObject();

        google->setRequestedScopeTokens({
            "https://www.googleapis.com/auth/classroom.coursework.students",
            "https://www.googleapis.com/auth/classroom.coursework.students.readonly ",
            "https://www.googleapis.com/auth/drive.readonly",
            "https://www.googleapis.com/auth/classroom.courses",
        });

        google->setClientIdentifier(json["client_id"].toString());
        google->setToken(json["client_secret"].toString());
        google->setAuthorizationUrl(QUrl(json["auth_uri"].toString()));
        google->setTokenUrl(QUrl(json["token_uri"].toString()));
    }

    bool NetworkManager::isConnected() const {
        return connectedStatus;
    }

    void NetworkManager::Authenticate() const {
        google->grant();
    }

    void NetworkManager::getListCourses() {
        if (!isConnected()) { emit requestFailed("Not authenticated"); }

        QNetworkRequest request(QUrl("https://classroom.googleapis.com/v1/courses"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", "Bearer " + google->token().toLatin1());
        QNetworkReply *reply = manager->get(request) ;
        connect(reply, &QNetworkReply::finished, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
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

            emit coursesReceived(document.object()["courses"].toArray());
        });

    }
} // Network
