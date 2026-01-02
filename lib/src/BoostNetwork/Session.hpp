#pragma once

#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/string_body_fwd.hpp>

namespace Network {
    class Session {
    public:
        Session(boost::asio::io_context &ioc);

        boost::asio::awaitable<void> connectToSender(const std::string &host);

        boost::asio::awaitable<void> stopConnectToSender();

        template<typename Body>
        boost::asio::awaitable<boost::beast::http::response<boost::beast::http::dynamic_body>> sendRequest(
            boost::beast::http::request<Body> req);


    private:
        boost::asio::ssl::context ctx_{boost::asio::ssl::context::tlsv12_client};
        boost::asio::ip::tcp::resolver resolver_;
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream_;
    };
} // Network
