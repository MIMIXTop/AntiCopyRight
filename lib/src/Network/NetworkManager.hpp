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

namespace Network {

    enum class ConnectionState {
        Connected,
        Connecting,
        Disconnecting,
        Disconnected
    };

    class NetworkManager : public QObject {
        Q_OBJECT

    public:
        static NetworkManager* GetInstance();
        NetworkManager(const NetworkManager&) = delete;
        NetworkManager& operator=(const NetworkManager&) = delete;


        void authenticate() const;

        /// Public API
        void getListCourses();

        void getStudentsWorks(const QString &coursId, const QString &coursWorkId);

        void downloadStudentWork(const QString &fileName, const QString &fileUrl);

        void getListCoursesWorks(const QJsonObject &course);

        void setConnectionState(ConnectionState state);

        ConnectionState getConnectionState() const { return connectedStatus; }
    signals:
        void requestFailed(const QString &message);

        void responseToRequest(ReplyTypes::Reply reply);

        void statusChanged(ConnectionState state);

        void isConnect();

    private:

        explicit NetworkManager(QObject *parent = nullptr);
        ///Methods for pending
        void enqueueWhenConnecting(const QString &name, quint64 timoutMs, std::function<void()> action);

        void drainPendingConnecting();

        void failAllPending(const QString &reason);

        ///Private API
        void startCoursesRequest();

        void startStudentsWorksRequest(const QString &courseId, const QString &courseWorkId);

        void startCourseWorksRequest(const QJsonObject &courseWorks);

        void startDownloadStudentWorksRequest(const QString &fileName, const QString &fileId);

        struct PendingAction {
            quint64 Id;
            QString name;
            std::function<void()> action;
            QTimer *timer{nullptr};
        };

        QOAuth2AuthorizationCodeFlow *google;
        QOAuthHttpServerReplyHandler *replyHandler;
        QNetworkAccessManager *manager;

        ConnectionState connectedStatus = ConnectionState::Disconnected;
        QHash<quint64, PendingAction> queueActions;
        quint64 NextId = 1;

#ifdef NETWORKMANAGER_TESTS
        friend struct NetworkManagerTestAccessor;
#endif
    };
#ifdef NETWORKMANAGER_TESTS
    struct NetworkManagerTestAccessor {
        static QNetworkAccessManager*& networkAccessManager(NetworkManager& n) { return n.manager; }
        static QOAuth2AuthorizationCodeFlow*& oauth(NetworkManager& n) { return n.google; }
        static ConnectionState& state(NetworkManager& n) { return n.connectedStatus; }
    };
#endif

} // Network
