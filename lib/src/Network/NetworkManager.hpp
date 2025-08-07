#pragma once

#include <Util/util.hpp>

#include <QObject>
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>
#include <QNetworkRequestFactory>
#include <QOAuthHttpServerReplyHandler>
#include <QString>

#include <expected>

namespace Network {
    class NetworkManager : public QObject {
        Q_OBJECT

    public:
        NetworkManager();

        ~NetworkManager() override {
            delete google;
            delete replyHandler;
            delete manager;
        }

        [[nodiscard]] bool isConnected() const;

        void Authenticate() const;

    signals:
        void coursesReceived(QJsonArray courses);
        void requestFailed(QString error);

    private:
        QOAuth2AuthorizationCodeFlow *google;
        QOAuthHttpServerReplyHandler *replyHandler;
        QNetworkRequestFactory apiClassroom{{"https://classroom.googleapis.com/v1"}};
        QNetworkRequestFactory apiDrive{{"https://www.googleapis.com/v1/drive/v2"}};
        QNetworkAccessManager *manager;
        bool connectedStatus = false;

        enum class REPLY_TYPE {
            COURSES,
            SOLUTIONS,
            WORKS,
        };

        void getListCourses();

        void getListCoursesWorks(QJsonArray);

        void getStudentsWorks(QString coursId, QString coursWorkId);

        void downloadStudentWork(QString fileName, QString fileUrl);
    };
} // Network
