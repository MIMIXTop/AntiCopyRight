#include <QtTest/QtTest>
#include <QNetworkAccessManager>
#include <QString>
#include <Network/ReplyTypes.hpp>

#include <Network/NetworkManager.hpp>

class FakeReply final : public QNetworkReply {
    Q_OBJECT

public:
    FakeReply(const QNetworkRequest &request,
              const QByteArray &payload,
              QNetworkReply::NetworkError error,
              const QString &errStr,
              QObject *parent = nullptr)
        : QNetworkReply(parent),
          data(payload) {
        setRequest(request);
        setUrl(request.url());
        setOpenMode(QIODevice::ReadOnly);
        if (!request.url().toString().contains("alt=media")) {
            setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        }
        if (error != QNetworkReply::NoError) {
            setError(error, errStr);
        }
        QMetaObject::invokeMethod(this, "emitReadyReadAndFinished", Qt::QueuedConnection);
    }

    void abort() override {
    }

    qint64 bytesAvailable() const override {
        return data.size() - offset + QNetworkReply::bytesAvailable();
    }

protected:
    qint64 readData(char *buffer, qint64 maxlen) override {
        if (offset >= data.size()) return -1;
        qint64 len = qMin<qint64>(maxlen, data.size() - offset);
        memcpy(buffer, data.constData() + offset, len);
        offset += len;
        return len;
    }

private slots:
    void emitReadyReadAndFinished() {
        if (error() == QNetworkReply::NoError && !data.isEmpty()) {
            emit readyRead();
        }
        emit finished();
    }

private:
    QByteArray data;
    quint64 offset = 0;
};

class FakeNetworkAccessManager final : public QNetworkAccessManager {
    Q_OBJECT

public:
    explicit FakeNetworkAccessManager(QObject *parent = nullptr)
        : QNetworkAccessManager(parent) {
    }

    void setNextResponse(const QByteArray &payload,
                         QNetworkReply::NetworkError error = QNetworkReply::NoError,
                         const QString &errStr = QString()) {
        nextPayload = payload;
        nextErrorString = errStr;
        nextError = error;
    }

    QNetworkRequest lastRequest;
    Operation lastOperation = QNetworkAccessManager::GetOperation;

protected:
    QNetworkReply *createRequest(Operation operation, const QNetworkRequest &request, QIODevice *) override {
        lastOperation = operation;
        lastRequest = request;
        return new FakeReply(request, nextPayload, nextError, nextErrorString, this);
    }

private:
    QByteArray nextPayload;
    QNetworkReply::NetworkError nextError = QNetworkReply::NoError;
    QString nextErrorString;
};


class NetworkManagerTest : public QObject {
    Q_OBJECT

private slots:
    void init() {
        networkManager = new Network::NetworkManager();
        delete networkManager->manager;

        fakeNetworkAccessManager = new FakeNetworkAccessManager(networkManager);
        networkManager->manager = fakeNetworkAccessManager;

        networkManager->google->setToken("TEST_TOKEN");
        networkManager->connectedStatus = Network::ConnectionState::Connected;
    }

    void cleanup() {
        delete networkManager;
        networkManager = nullptr;
        fakeNetworkAccessManager = nullptr;
    }

    void statusChanged_emits_on_chnge() {
        QSignalSpy spy(networkManager, SIGNAL(statusChanged(Network::ConnectionState)));
        networkManager->setConnectionState(Network::ConnectionState::Connecting);
        QCOMPARE(spy.count(), 1);
        networkManager->setConnectionState(Network::ConnectionState::Connecting);
        QCOMPARE(spy.count(), 1);
    }

    void getListCourses_success_response() {
        QJsonArray courses;
        courses.append(QJsonObject{{"id", "C1"}});
        QByteArray payload = QJsonDocument(QJsonObject{{"courses", courses}}).toJson();

        fakeNetworkAccessManager->setNextResponse(payload);
        QSignalSpy ok(networkManager, SIGNAL(Network::NetworkManager::responseToRequest(ReplyTypes::Type)));
        QSignalSpy err(networkManager, SIGNAL(Network::NetworkManager::requestFailed(QString)));

        networkManager->setConnectionState(Network::ConnectionState::Connecting);
        networkManager->getListCourses();

        QTRY_COMPARE(ok.count(), 1);
        QCOMPARE(err.count(), 0);

        QCOMPARE(fakeNetworkAccessManager->lastOperation, QNetworkAccessManager::GetOperation);
        QCOMPARE(fakeNetworkAccessManager->lastRequest.url().toString(), QString("https://classroom.googleapis.com/v1/courses"));
        QVERIFY(fakeNetworkAccessManager->lastRequest.rawHeader("Authorization").startsWith("Bearer "));
        }

    void getListCourses_network_error_emits_requestFailed() {
        fakeNetworkAccessManager->setNextResponse(QByteArray(), QNetworkReply::HostNotFoundError, "Host not found");

        QSignalSpy ok(networkManager, SIGNAL(Network::NetworkManager::responseToRequestFailed(ReplyTypes::Type)));
        QSignalSpy err(networkManager, SIGNAL(Network::NetworkManager::requestFailed(QString)));
        networkManager->setConnectionState(Network::ConnectionState::Connecting);
        networkManager->getListCourses();

        QTRY_COMPARE(err.count(), 1);
        QCOMPARE(ok.count(), 0);
        QVERIFY(err.takeFirst().at(0).toString().contains("Network error"));
    }

    void getListCourses_invalid_json_emits_requestFailed() {
        QByteArray payload = QJsonDocument(QJsonObject{{"foo", 1}}).toJson();
        fakeNetworkAccessManager->setNextResponse(payload);

        QSignalSpy ok(networkManager, SIGNAL(Network::NetworkManager::responseToRequest(ReplyTypes::Type)));
        QSignalSpy err(networkManager, SIGNAL(Network::NetworkManager::requestFailed(QString)));

        networkManager->setConnectionState(Network::ConnectionState::Connecting);
        networkManager->getListCourses();

        QTRY_COMPARE(err.count(), 1);
        QCOMPARE(ok.count(), 0);
        QVERIFY(err.takeFirst().at(0).toString().contains("Invalid JSON response"));
    }

    void downloadStudentWork_emits_payload() {
        QByteArray fileData = "LOLOLO";
        fakeNetworkAccessManager->setNextResponse(fileData);
        QSignalSpy ok(networkManager, SIGNAL(Network::NetworkManager::responseToRequest(ReplyTypes::Type)));
        QSignalSpy err(networkManager, SIGNAL(Network::NetworkManager::requestFailed(QString)));

        networkManager->setConnectionState(Network::ConnectionState::Connecting);
        networkManager->downloadStudentWork("report.txt", "ID_123");

        QTRY_COMPARE(ok.count(), 1);
        QCOMPARE(err.count(), 0);

        QCOMPARE(fakeNetworkAccessManager->lastRequest.url().toString(),
            QString("https://www.googleapis.com/drive/v3/files/ID_123?alt=media"));
        QVERIFY(fakeNetworkAccessManager->lastRequest.rawHeader("Authorization").startsWith("Bearer "));
    }

    void enqueue_when_Disconnecting_emits_NotAuthenticated() {
        QSignalSpy err(networkManager, SIGNAL(Network::NetworkManager::responseToRequest(ReplyTypes::Type)));

        networkManager->setConnectionState(Network::ConnectionState::Disconnecting);
        networkManager->getListCourses();

        QTRY_COMPARE(err.count(), 1);
        QVERIFY(err.takeFirst().at(0).toString().contains("Not authenticated"));
    }

    void enqueue_then_Disconnecting_fails_pending() {
        networkManager->setConnectionState(Network::ConnectionState::Connected);

        QSignalSpy err(networkManager, SIGNAL(Network::NetworkManager::requestFailed(QString)));

        networkManager->getListCourses();

        networkManager->setConnectionState(Network::ConnectionState::Disconnecting);
        QTRY_COMPARE(err.count(), 1);
        QVERIFY(err.takeFirst().at(0).toString().contains("Not authenticated"));
    }

    void pending_should_drain_on_Connecting_but_currently_does_not() {
        networkManager->setConnectionState(Network::ConnectionState::Connected);

        QJsonArray courses;
        courses.append(QJsonObject{{"id", "C1"}});

        QByteArray payload = QJsonDocument(QJsonObject{{"courses", courses}}).toJson();
        fakeNetworkAccessManager->setNextResponse(payload);

        QSignalSpy ok(networkManager, SIGNAL(Network::NetworkManager::responseToRequest(ReplyTypes::Type)));
        networkManager->getListCourses();

        QEXPECT_FAIL("", "drainPendingConnecting имеет инвертированную проверку и не дренит при Connecting", Continue);
        networkManager->setConnectionState(Network::ConnectionState::Connecting);

        QTest::qWait(50);
        QCOMPARE(ok.count(), 1);
    }

private :
    Network::NetworkManager *networkManager = nullptr;
    FakeNetworkAccessManager *fakeNetworkAccessManager = nullptr;
};

QTEST_MAIN(NetworkManagerTest)
#include "QtNetworkTests.moc"
