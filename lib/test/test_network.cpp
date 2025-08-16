#include  <gtest/gtest.h>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSignalSpy>
#include <QTest>

#include <Network/NetworkManager.hpp>


using Accessor = Network::NetworkManagerTestAccessor;


#include <chrono>
#include <thread>
class FakeReply final : public QNetworkReply {
    Q_OBJECT

public:
    FakeReply(const QNetworkRequest &request,
              const QByteArray &payload,
              QNetworkReply::NetworkError error,
              const QString &errorStr,
              QObject *parent = nullptr)
        : QNetworkReply(parent),
          data(payload) {
        setRequest(request);
        setUrl(request.url());
        setOpenMode(QIODevice::ReadOnly);
        if (!request.url().toString().contains("alt=media")) {
            setHeader(QNetworkRequest::ContentLengthHeader, "application/json");
        }
        if (error != QNetworkReply::NoError) {
            setError(error, errorStr);
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
        memcpy(buffer, data.constData() + offset, std::size_t(len));
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
    qint64 offset = 0;
};

class FakeNetworkAccessManager final : public QNetworkAccessManager {
    Q_OBJECT

public:
    explicit FakeNetworkAccessManager(QObject *parent = nullptr)
        : QNetworkAccessManager(parent) {
    }

    void setNextRespons(const QByteArray &payload, QNetworkReply::NetworkError error = QNetworkReply::NoError,
                        const QString &errorStr = QString()) {
        nextPayload = payload;
        nextErrorString = errorStr;
        nextError = error;
    }

    QNetworkRequest lastRequest;
    Operation lastOp = QNetworkAccessManager::GetOperation;

protected:
    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *) override {
        lastOp = op;
        lastRequest = request;
        return new FakeReply(request, nextPayload, nextError, nextErrorString, this);
    }

private:
    QByteArray nextPayload;
    QNetworkReply::NetworkError nextError = QNetworkReply::NoError;
    QString nextErrorString;
};

class NetworkManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        delete Accessor::networkAccessManager(*nm);
        fakeNm = new FakeNetworkAccessManager(nm);

        Accessor::networkAccessManager(*nm) = fakeNm;
        Accessor::oauth(*nm)->setToken("TEST_TOKET");
        Accessor::state(*nm) = Network::ConnectionState::Connected;
    }

    void TearDown() override {
        fakeNm = nullptr;
    }

    Network::NetworkManager *nm = Network::NetworkManager::GetInstance();
    FakeNetworkAccessManager *fakeNm = nullptr;
};

TEST_F(NetworkManagerTest, StatusChanged_emits_on_change) {
    QSignalSpy spy(nm, &Network::NetworkManager::statusChanged);
    nm->setConnectionState(Network::ConnectionState::Connecting);
    EXPECT_EQ(spy.count(), 1);
    nm->setConnectionState(Network::ConnectionState::Connecting); // повтор
    EXPECT_EQ(spy.count(), 1); // не должен эмитить при том же состоянии
}

TEST_F(NetworkManagerTest, GetListCourses_success_response) {
    QJsonArray courses;
    courses.append(QJsonObject{{"id", "C1"}});
    QByteArray payload = QJsonDocument(QJsonObject{{"courses", courses}}).toJson();

    fakeNm->setNextRespons(payload);

    QSignalSpy okSpy(nm, &Network::NetworkManager::responseToRequest);
    QSignalSpy errSpy(nm, &Network::NetworkManager::requestFailed);

    // При состоянии Connecting action выполняется сразу
    nm->setConnectionState(Network::ConnectionState::Connecting);
    nm->getListCourses();

    QTRY_COMPARE(okSpy.count(), 1);
    EXPECT_EQ(errSpy.count(), 0);

    // Проверим URL и заголовок авторизации
    EXPECT_EQ(fakeNm->lastOp, QNetworkAccessManager::GetOperation);
    EXPECT_EQ(fakeNm->lastRequest.url().toString(), QString("https://classroom.googleapis.com/v1/courses"));
    EXPECT_TRUE(fakeNm->lastRequest.rawHeader("Authorization").startsWith("Bearer "));
}

TEST_F(NetworkManagerTest, GetListCourses_network_error_emits_requestFailed) {
    fakeNm->setNextRespons(QByteArray(), QNetworkReply::HostNotFoundError, "Host not found");

    QSignalSpy okSpy(nm, &Network::NetworkManager::responseToRequest);
    QSignalSpy errSpy(nm, &Network::NetworkManager::requestFailed);

    nm->setConnectionState(Network::ConnectionState::Connecting);
    nm->getListCourses();

    QTRY_COMPARE(errSpy.count(), 1);
    EXPECT_EQ(okSpy.count(), 0);
    EXPECT_TRUE(errSpy.takeFirst().at(0).toString().contains("Network error"));
}

TEST_F(NetworkManagerTest, GetListCourses_invalid_json_emits_requestFailed) {
    // Нет ключа "courses"
    QByteArray payload = QJsonDocument(QJsonObject{{"foo", 1}}).toJson();
    fakeNm->setNextRespons(payload);

    QSignalSpy okSpy(nm, &Network::NetworkManager::responseToRequest);
    QSignalSpy errSpy(nm, &Network::NetworkManager::requestFailed);

    nm->setConnectionState(Network::ConnectionState::Connecting);
    nm->getListCourses();

    QTRY_COMPARE(errSpy.count(), 1);
    EXPECT_EQ(okSpy.count(), 0);
    EXPECT_TRUE(errSpy.takeFirst().at(0).toString().contains("Invalid JSON response"));
}

TEST_F(NetworkManagerTest, GetStudentsWorks_success_response) {
    QJsonArray subs;
    subs.append(QJsonObject{{"userId", "U1"}});
    QByteArray payload = QJsonDocument(QJsonObject{{"studentSubmissions", subs}}).toJson();
    fakeNm->setNextRespons(payload);

    QSignalSpy okSpy(nm, &Network::NetworkManager::responseToRequest);
    QSignalSpy errSpy(nm, &Network::NetworkManager::requestFailed);

    nm->setConnectionState(Network::ConnectionState::Connecting);
    nm->getStudentsWorks("COURSE_X", "WORK_Y");

    QTRY_COMPARE(okSpy.count(), 1);
    EXPECT_EQ(errSpy.count(), 0);
    EXPECT_EQ(fakeNm->lastRequest.url().toString(),
        QString("https://classroom.googleapis.com/v1/courses/COURSE_X/courseWork/WORK_Y/studentSubmissions"));
}

TEST_F(NetworkManagerTest, GetListCoursesWorks_uses_second_course_id) {
    QJsonArray cw;
    cw.append(QJsonObject{{"id", "W1"}});
    QByteArray payload = QJsonDocument(QJsonObject{{"courseWork", cw}}).toJson();
    fakeNm->setNextRespons(payload);

    QJsonArray courses;
    courses.append(QJsonObject{{"id", "C1"}});
    courses.append(QJsonObject{{"id", "C2"}}); // должен быть использован index 1

    QSignalSpy okSpy(nm, &Network::NetworkManager::responseToRequest);
    QSignalSpy errSpy(nm, &Network::NetworkManager::requestFailed);

    nm->setConnectionState(Network::ConnectionState::Connecting);
    nm->getListCoursesWorks(courses[1].toObject());

    QTRY_COMPARE(okSpy.count(), 1);
    EXPECT_EQ(errSpy.count(), 0);
    EXPECT_EQ(fakeNm->lastRequest.url().toString(),
        QString("https://classroom.googleapis.com/v1/courses/C2/courseWork"));
}

TEST_F(NetworkManagerTest, DownloadStudentWork_emits_payload) {
    QByteArray fileData = "FILEDATA";
    fakeNm->setNextRespons(fileData);

    QSignalSpy okSpy(nm, &Network::NetworkManager::responseToRequest);
    QSignalSpy errSpy(nm, &Network::NetworkManager::requestFailed);

    nm->setConnectionState(Network::ConnectionState::Connecting);
    nm->downloadStudentWork("report.txt", "ID_123");

    QTRY_COMPARE(okSpy.count(), 1);
    EXPECT_EQ(errSpy.count(), 0);
    EXPECT_EQ(fakeNm->lastRequest.url().toString(),
        QString("https://www.googleapis.com/drive/v3/files/ID_123?alt=media"));
    EXPECT_TRUE(fakeNm->lastRequest.rawHeader("Authorization").startsWith("Bearer "));
}

TEST_F(NetworkManagerTest, Enqueue_when_Disconnecting_emits_NotAuthenticated) {
    QSignalSpy errSpy(nm, &Network::NetworkManager::requestFailed);

    nm->setConnectionState(Network::ConnectionState::Disconnecting);
    nm->getListCourses(); // В enqueueWhenConnecting при Disconnecting — сразу fail

    QTRY_COMPARE(errSpy.count(), 1);
    EXPECT_TRUE(errSpy.takeFirst().at(0).toString().contains("Not authenticated"));
}

TEST_F(NetworkManagerTest, Enqueue_then_Disconnecting_fails_pending) {
    // Сначала поставим состояние не-Connecting/не-Disconnecting (например, Connected)
    nm->setConnectionState(Network::ConnectionState::Connected);

    QSignalSpy errSpy(nm, &Network::NetworkManager::requestFailed);

    // Заявка попадет в очередь
    nm->getListCourses();

    // Теперь "обрываем" — должны зафейлить все pending
    nm->setConnectionState(Network::ConnectionState::Disconnecting);

    QTRY_COMPARE(errSpy.count(), 1);
    EXPECT_TRUE(errSpy.takeFirst().at(0).toString().contains("Not authenticated"));
}
#include "test_network.moc"
