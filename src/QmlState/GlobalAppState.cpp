#include "GlobalAppState.hpp"

#include <exception>
#include <variant>

#include "Network/ReplyType.hpp"
#include "Util/util.hpp"

void GlobalState::updateCource() {
    manager->getCourses([this](ReplyTypes::Reply reply) {
        std::visit(
            util::match {
              [this](ReplyTypes::Types::Course replay) {},
              [this](ReplyTypes::Types::Error error) {

              },
              [this](auto reply) { std::terminate(); } },
            reply);
    });
}