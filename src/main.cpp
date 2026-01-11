#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <print>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    std::println("https://youtu.be/xvFZjo5PgG0?si=AkVUMST4gQ_NMMu4");

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() {
            qCritical() << "Failed to create QML object!";
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [](QObject* obj, const QUrl& url) {
            if (!obj) {
                qCritical() << "Object is null! URL:" << url;
                QCoreApplication::exit(-1);
            } else {
                qDebug() << "QML object created successfully:" << obj;
                // Принудительно показываем окно
                QQuickWindow* window = qobject_cast<QQuickWindow*>(obj);
                if (window) {
                    window->show();
                }
            }
        },
        Qt::QueuedConnection);

    engine.loadFromModule("QML_SRC", "MainWindow");

    return app.exec();
}
