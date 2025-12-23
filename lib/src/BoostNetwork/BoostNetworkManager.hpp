#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <string>
#include <vector>
#include <concepts>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/http/basic_parser.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
// namespace json = boost::json;
using tcp = asio::ip::tcp;

template <class T>
concept StringConteiner = requires(T t) {
    typename T::value_type;
    requires std::same_as<typename T::value_type, std::string>;
    { t.begin() } -> std::input_iterator;
    { t.end() } -> std::input_iterator;
};

class AuthenticationManager {
public:
    AuthenticationManager(std::vector<std::string> scopes = getDefaultScope(), int port = 8080);

    void run_server();

    ~AuthenticationManager();

    std::string getAuthUrl();

    bool EmptyAccessToken();

    bool EmptyRefreshToken();

    std::string_view getToken();

private:
    struct {
        const std::string package = "com.example.AntyCopyRigtht";
        const std::string service = "authentication-sevice";
        const std::string user = "Admin";
        std::string clientId;
        std::string clientSecret;
        std::string accessToken;
        std::string refreshToken;
        std::string code;
        std::string scope;
    } config_;

    enum class RequestStatus{
        GET_TOKEN,
        UPDATE_TOKENS,
    };

    void load_config(const std::vector<std::string> &scopes);

    static std::vector<std::string> getDefaultScope();

    void SaveRefreshToken(const std::string &refreshToken) const;

    void handleRequest(http::request<http::string_body> &req, RequestStatus status);

    asio::awaitable<void> listen();

    asio::awaitable<void> workWithClient(tcp::socket socket);

    asio::awaitable<http::response<http::string_body>> sender(http::request<http::string_body> &req);

    asio::awaitable<void> getTokens();

    void handleGetTokens(std::string_view boby);

    std::string extractCode(std::string_view code);

    void updateTokens(const boost::system::error_code& error);

    asio::io_context ioc;
    asio::steady_timer timer;
    asio::ssl::context ctx;
    std::string host_ = "127.0.0.1";
    unsigned short port_;
};
