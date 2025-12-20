#pragma once

#include <string>
#include <vector>
#include <concepts>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/http/basic_parser.hpp>

namespace asio = boost::asio;
// namespace beast = boost::beast;
// namespace http = beast::http;
// namespace json = boost::json;
using tcp = asio::ip::tcp;

template <class T>
concept StringConteiner = requires(T t) {
    typename T::value_type;
    requires std::same_as<typename T::value_type, std::string>;
    { t.begin() } -> std::input_iterator;
    { t.end() } -> std::input_iterator;
};

class Server {
public:
    Server(std::vector<std::string> scopes = getDefaultScope(), int port = 8080);

    void run_server();

    ~Server();

    std::string getAuthUrl();

    bool EmptyAccessToken();
    bool EmptyRefreshToken();

private:
    struct {
        std::string clientId;
        std::string clientSecret;
        std::string accessToken;
        std::string refreshToken;
        std::string code;
        std::string scope;
    } config_;

    void load_config(const std::vector<std::string> &scopes);

    static std::vector<std::string> getDefaultScope();

    void handleRequest();

    asio::awaitable<void> listen();
    asio::awaitable<void> workWithClient(tcp::socket socket);
    asio::awaitable<void> getTokens();

    void handleGetTokens(std::string_view boby);

    std::string extractCode(std::string_view code);

    asio::io_context ioc;
    asio::ssl::context ctx;
    std::string host_ = "127.0.0.1";
    unsigned short port_;
};
