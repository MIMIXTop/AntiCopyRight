#pragma once

#include <string>
#include <vector>
#include <concepts>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

template <class T>
concept StringConteiner = requires(T t) {
    typename T::value_type;
    requires std::same_as<typename T::value_type, std::string>;
    { t.begin() } -> std::input_iterator;
    { t.end() } -> std::input_iterator;
};

namespace Network {
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
            std::string authUri;
            std::string tokenUri;
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

        void handleRequest(boost::beast::http::request<boost::beast::http::string_body> &req, RequestStatus status);

        boost::asio::awaitable<void> listen();

        boost::asio::awaitable<void> workWithClient(boost::asio::ip::tcp::socket socket);

        boost::asio::awaitable<boost::beast::http::response<boost::beast::http::string_body>> sender(boost::beast::http::request<boost::beast::http::string_body> &req);

        boost::asio::awaitable<void> getTokens();

        void handleGetTokens(std::string_view boby);

        std::string extractCode(std::string_view code);

        void updateTokens(const boost::system::error_code& error);

        boost::asio::io_context ioc;
        boost::asio::steady_timer timer;
        boost::asio::ssl::context ctx;
        std::string host_ = "127.0.0.1";
        unsigned short port_;
    };
}