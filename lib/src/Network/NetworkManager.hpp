#pragma once

#include  "ReplyTypes.hpp"

#include <QObject>
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>
#include <QNetworkRequestFactory>
#include <QOAuthHttpServerReplyHandler>
#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QHash>
#include <QTimer>

#include <functional>

#ifdef NETWORKMANAGER_TESTS
class NetworkManagerTest;
#endif

namespace Network {
    enum class ConnectionState {
        Connected,
        Connecting,
        Disconnecting,
        Disconnected
    };


    class NetworkManager final : public QObject {
        Q_OBJECT

    public:

        explicit NetworkManager(QObject *parent = nullptr);

        ~NetworkManager() override = default;

        void authenticate() const;

        /// Public API
        void getListCourses();

        void getStudentsWorks(const QString &coursId, const QString &coursWorkId);

        void downloadStudentWork(const QString &fileName, const QString &fileUrl);

        void getListCoursesWorks(const QJsonArray &courses);

        void setConnectionState(ConnectionState state);

        [[nodiscard]] ConnectionState getConnectionState() const { return connectedStatus; }
    signals:
        void requestFailed(const QString &message);

        void responseToRequest(ReplyTypes::Reply reply);

        void statusChanged(ConnectionState state);

    private:
        ///Methods for pending
        void enqueueWhenConnecting(const QString &name, quint64 timoutMs, std::function<void()> action);

        void drainPendingConnecting();

        void failAllPending(const QString &reason);

        ///Private API
        void startCoursesRequest();

        void startStudentsWorksRequest(const QString &courseId, const QString &courseWorkId);

        void startCourseWorksRequest(const QJsonArray &courseWorks);

        void startDownloadStudentWorksRequest(const QString &fileName, const QString &fileId);

        struct PendingAction {
            quint64 Id;
            QString name;
            std::function<void()> action;
            QTimer *timer{nullptr};
        };

        QOAuth2AuthorizationCodeFlow *google;
        QOAuthHttpServerReplyHandler *replyHandler;
        QNetworkRequestFactory apiClassroom{{"https://classroom.googleapis.com/v1"}};
        QNetworkRequestFactory apiDrive{{"https://www.googleapis.com/v1/drive/v2"}};
        QNetworkAccessManager *manager;

        ConnectionState connectedStatus = ConnectionState::Connected;
        QHash<quint64, PendingAction> queueActions;
        quint64 NextId = 1;

#ifdef NETWORKMANAGER_TESTS
    friend class NetworkManagerTest;
#endif
    };
} // Network
