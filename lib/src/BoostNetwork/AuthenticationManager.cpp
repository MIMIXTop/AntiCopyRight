#include "AuthenticationManager.hpp"
#include <keychain/keychain.h>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <boost/system/detail/error_code.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include <Config/ConfigParser.hpp>

namespace {
template <StringConteiner... Conteiners>
std::string foldString(const Conteiners&... conteiners) {
  std::ostringstream oss;
  auto append = [&](const auto& c) {
    for (auto&& s : c) {
      oss << s << " ";
    }
  };
  (append(conteiners), ...);
  return oss.str();
}
}  // namespace

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace json = boost::json;
namespace http = beast::http;
using tcp = asio::ip::tcp;

namespace Network {
AuthenticationManager::AuthenticationManager(std::vector<std::string> scopes,
                                             int port)
    : port_(port), ctx(asio::ssl::context::tls_client), timer(ioc) {
  load_config(scopes);
  keychain::Error error;
  std::string token = keychain::getPassword(config_.package, config_.service,
                                            config_.user, error);
  if (error.message != "") {
    boost::system::error_code code;
    updateTokens(code);
  }
}

AuthenticationManager::~AuthenticationManager() {}

std::string AuthenticationManager::getAuthUrl() {
  std::string authUrl = "https://accounts.google.com/o/oauth2/auth&";
  authUrl += std::format("scope={}&", config_.scope);
  authUrl += std::format("response_type=code&");
  authUrl += std::format("redirect_uri=http://{}:{}/code&", host_, port_);
  authUrl += std::format("client_id={}", config_.clientId);
  return authUrl;
}

bool AuthenticationManager::EmptyAccessToken() const {
  return config_.accessToken.empty();
}

bool AuthenticationManager::EmptyRefreshToken() {
  return config_.refreshToken.empty();
}

void AuthenticationManager::updateTokens(
    const boost::system::error_code& error) {
  if (!error) {
    asio::co_spawn(
        ioc,
        [this]() -> asio::awaitable<void> {
          try {
            http::request<http::string_body> req;
            handleRequest(req, RequestStatus::UPDATE_TOKENS);

            auto res = co_await sender(req);
            handleGetTokens(res.body());
          } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
          }
        },
        asio::detached);
  }
}

void AuthenticationManager::load_config(const std::vector<std::string>& scopes) {
  auto parser = Util::ConfigParser();
  config_.clientId = parser["CLIENT_ID"];
  config_.clientSecret = parser["CLIENT_SECRET"];
  config_.authUri = parser["AUTH_URI"];
  config_.tokenUri = parser["TOKEN_URI"];
  config_.scope = foldString(scopes);
}

std::vector<std::string> AuthenticationManager::getDefaultScope() {
  return {
      "https://www.googleapis.com/auth/classroom.coursework.students",
      "https://www.googleapis.com/auth/classroom.coursework.students.readonly",
      "https://www.googleapis.com/auth/drive.readonly",
      "https://www.googleapis.com/auth/classroom.courses",
      "https://www.googleapis.com/auth/classroom.rosters.readonly"};
}

void AuthenticationManager::SaveRefreshToken(
    const std::string& refreshToken) const {
  keychain::Error error;
  keychain::setPassword(config_.package, config_.service, config_.user,
                        refreshToken, error);
  if (error) {
    std::cerr << error.message << std::endl;
    throw std::runtime_error(error.message);
  }
}

void AuthenticationManager::run_server() {
  asio::signal_set signals{ioc, SIGINT, SIGTERM};
  signals.async_wait([this](auto, auto) { ioc.stop(); });
  asio::co_spawn(ioc, listen(), asio::detached);
  ioc.run();
}

asio::awaitable<void> AuthenticationManager::listen() {
  const auto executor = co_await asio::this_coro::executor;

  tcp::acceptor acceptor{executor};
  tcp::endpoint endpoint{asio::ip::tcp::v4(), port_};

  acceptor.open(endpoint.protocol());
  acceptor.set_option(tcp::acceptor::reuse_address(true));
  acceptor.bind(endpoint);
  acceptor.listen();

  while (true) {
    auto socket = co_await acceptor.async_accept(asio::use_awaitable);
    asio::co_spawn(ioc, workWithClient(std::move(socket)), asio::detached);
  }
}

asio::awaitable<void>
AuthenticationManager::workWithClient(tcp::socket socket) {
  beast::flat_buffer buffer;
  http::request<http::string_body> req;

  co_await http::async_read(socket, buffer, req, asio::use_awaitable);

  if (auto authCode = extractCode(req.target()); !authCode.empty()) {
    config_.code = authCode;
    co_await getTokens();

    http::response<http::string_body> res;
    res.set(http::field::content_type, "text/html");
    res.body() = "Authorization successful! You can close this tab.";
    res.prepare_payload();
    co_await http::async_write(socket, res, asio::use_awaitable);
  }
}

asio::awaitable<void> AuthenticationManager::getTokens() {
  try {
    http::request<http::string_body> req;
    handleRequest(req, RequestStatus::GET_TOKEN);

    http::response<http::string_body> res = co_await sender(req);

    handleGetTokens(res.body());
  } catch (const std::exception& e) {
    std::cerr << "Server error(get tokent)" << e.what() << std::endl;
  }
}

void AuthenticationManager::handleGetTokens(std::string_view boby) {
  auto resBody = json::parse(boby).as_object();

  config_.accessToken = resBody["access_token"].as_string();

  if (resBody.contains("refresh_token")) {
    SaveRefreshToken(resBody["refresh_token"].as_string().c_str());
  }
  timer.expires_after(std::chrono::seconds(resBody["expires_in"].as_int64()));
  timer.async_wait(std::bind(&AuthenticationManager::updateTokens, this,
                             std::placeholders::_1));
}

std::string AuthenticationManager::extractCode(std::string_view code) {
  auto pos = code.find("code=");
  if (pos == std::string::npos) {
    return "";
  }

  std::string_view codePart = code.substr(pos + 5);
  auto endPos = codePart.find('&');
  if (endPos != std::string::npos) {
    return std::string(codePart.substr(0, endPos));
  }
  return std::string(codePart);
}

void AuthenticationManager::handleRequest(http::request<http::string_body>& req,
                                          RequestStatus status) {
  std::string refreshToken;
  if (RequestStatus::GET_TOKEN == status) {
    keychain::Error error;
    refreshToken = keychain::getPassword(config_.package, config_.service,
                                         config_.user, error);

    if (refreshToken.empty() || error) {
      std::cout << error.message << std::endl;
      throw std::runtime_error(error.message);
    }
  }

  switch (status) {
  case RequestStatus::GET_TOKEN:
    req.method(http::verb::post);
    req.target("/token");
    req.version(11);
    req.set(http::field::host, "oauth2.googleapis.com");
    req.set(http::field::content_type, "application/x-www-form-urlencoded");
    req.set(http::field::accept, "application/json");
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.body() = "code=" + config_.code + "&" +
                 "client_id=" + config_.clientId + "&" +
                 "client_secret=" + config_.clientSecret + "&" +
                 "redirect_uri=http://127.0.0.1:8080/code&" +
                 "grant_type=authorization_code";
    break;
  case RequestStatus::UPDATE_TOKENS:
    req.method(http::verb::post);
    req.target("/token");
    req.version(11);
    req.set(http::field::content_type, "application/x-www-form-urlencoded");
    req.set(http::field::host, "oauth2.googleapis.com");
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::accept, "application/json");
    req.body() = "client_id=" + config_.clientId + "&" +
                 "client_secret=" + config_.clientSecret + "&" +
                 "refresh_token=" + refreshToken + "&" +
                 "grant_type=refresh_token";
    break;
  }
  req.prepare_payload();
}

std::string_view AuthenticationManager::getToken() const {
  return config_.accessToken;
}

asio::awaitable<http::response<http::string_body>>
AuthenticationManager::sender(http::request<http::string_body>& req) {
  asio::ssl::stream<tcp::socket> ssl_socket{ioc, ctx};
  tcp::resolver resolver{ioc};

  auto result = co_await resolver.async_resolve("oauth2.googleapis.com", "443",
                                                asio::use_awaitable);
  co_await asio::async_connect(ssl_socket.next_layer(), result,
                               asio::use_awaitable);
  co_await ssl_socket.async_handshake(asio::ssl::stream_base::client,
                                      asio::use_awaitable);

  co_await http::async_write(ssl_socket, req, asio::use_awaitable);
  beast::flat_buffer buffer;
  http::response<http::string_body> res;
  co_await http::async_read(ssl_socket, buffer, res, asio::use_awaitable);
  co_return res;
}
}  // namespace Network