#include <QFile>
#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>

#include <QmlModels/courselistmodel.hpp>
#include <QmlModels/coursworkslistmodel.hpp>
#include <QmlModels/proxymodel.hpp>

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;

  qmlRegisterType<CourseListModel>("MyModels", 1, 0, "CourseModel");
  qmlRegisterType<CoursWorksListModel>("MyModels", 1, 0, "WorksModel");
  qmlRegisterType<ProxyModel>("MyModels", 1, 0, "WorksByCoursProxy");

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.loadFromModule("QML_SRC", "Main");
  return app.exec();
}
