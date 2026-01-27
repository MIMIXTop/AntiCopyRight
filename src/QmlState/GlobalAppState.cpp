#include "GlobalAppState.hpp"
#include <qobjectdefs.h>
#include <exception>
#include <functional>
#include <thread>
#include <variant>

#include <QDesktopServices>
#include <QMessageBox>
#include <QMetaObject>
#include <QUrl>

#include "Network/ReplyType.hpp"
#include "Util/util.hpp"

void GlobalState::authUser() {
    QDesktopServices::openUrl(QString::fromStdString(manager->getAuthUrl()));
    initData();
}

void GlobalState::initData() {
    manager->getUserInfo(std::bind(&GlobalState::RequestHandler, this, std::placeholders::_1));
}

void GlobalState::RequestHandler(ReplyTypes::Reply reply) {
    QMetaObject::invokeMethod(this, [this, reply] {
        std::visit(
            util::match {
              [this](ReplyTypes::Types::Courses corses) {},
              [this](ReplyTypes::Types::CourseWorks corses) {},
              [this](ReplyTypes::Types::StudentWorks corses) {},
              [this](ReplyTypes::Types::StudentList corses) {},
              [this](ReplyTypes::Types::DownloadStudentWork corses) {},
              [this](ReplyTypes::Types::UserInfo info) { emit UpdateUserInfo(info.name.data(), info.photoUrl.data()); },
              [this](ReplyTypes::Types::Error corses) {},
            },
            reply);
    });
}