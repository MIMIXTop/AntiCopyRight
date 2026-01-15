#include "GlobalAppState.hpp"
#include <exception>
#include <thread>
#include <variant>

#include <QDesktopServices>
#include <QMessageBox>
#include <QMetaObject>
#include <QUrl>

#include "Network/ReplyType.hpp"
#include "Util/util.hpp"

void GlobalState::authUser() {
    manager->getUserInfo([this](ReplyTypes::Reply reply) {
        QMetaObject::invokeMethod(this, [this, reply]() {
            std::visit(
                util::match {
                  [this](ReplyTypes::Types::UserInfo info) {
                      emit UpdateUserInfo(info.name.data(), info.photoUrl.data());
                  },
                  [this](ReplyTypes::Types::Error error) {
                      QMessageBox msgBox;
                      msgBox.setText(error.errorMessage.c_str());
                      msgBox.exec();
                  },
                  [](auto) {} },
                reply);
        });
    });

    QDesktopServices::openUrl(QString::fromStdString(manager->getAuthUrl()));
}