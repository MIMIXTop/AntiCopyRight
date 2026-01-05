//
// Created by mimixtop on 26.12.2025.
//

#include "Session.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/empty_body.hpp>

#include "Util/util.hpp"

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

using tcp = asio::ip::tcp;

namespace Network {
    Session::Session(asio::io_context &ioc)
        : resolver_(ioc), stream_(ioc, ctx_) {
    }

    asio::awaitable<void> Session::connectToSender(const std::string &host) {
        auto endpoint = co_await resolver_.async_resolve(host, "443", asio::use_awaitable);
        co_await asio::async_connect(stream_.next_layer(), endpoint, asio::use_awaitable);
        co_await stream_.async_handshake(asio::ssl::stream_base::client, asio::use_awaitable);
    }

    asio::awaitable<void> Session::stopConnectToSender() {
        co_await stream_.async_shutdown(asio::use_awaitable);
        stream_.next_layer().close();
    }


    asio::awaitable<http::response<http::dynamic_body>> Session::sendRequest(http::request<http::empty_body> req) {
        co_await http::async_write(stream_, std::move(req), asio::use_awaitable);
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        co_await http::async_read(stream_, buffer, res, asio::use_awaitable);
        co_return res;
    }
} // Network
