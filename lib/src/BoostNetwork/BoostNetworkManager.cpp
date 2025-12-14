#include "BoostNetworkManager.hpp"
#include <ATen/core/interned_strings.h>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/json.hpp>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace {
#ifdef WIN32
#define CREDENTIALS_PATH "Util\\Network\\init.json"
#else
#define CREDENTIALS_PATH "../Utils/Network/init.json"
#endif
} // namespace

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
using tcp = asio::ip::tcp;

Server::Server(std::vector<std::string> scopes, int port)
    : port_(port) {
  ;
  load_config(scopes);
}

void Server::load_config(std::vector<std::string> scopes) {
  std::ifstream file(CREDENTIALS_PATH);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open config file");
  }
  std::string configFile((std::istreambuf_iterator(file)),
                         std::istreambuf_iterator<char>());
  json::object jsonFile = json::parse(configFile).as_object();

  config_.authUrl = "https://accounts.google.com/o/oauth2/v2/auth?";
  config_.clientId = jsonFile.at("installed").at("client_id").as_string();
  std::ranges::for_each(scopes, [this](auto &s) { config_.scope.append(s); });
  config_.redirectUrl = std::format("http://127.0.0.1:{}", port_);
  config_.responseType = "code";
}

std::vector<std::string> Server::getDefaultScope() {
  return {
      "https://www.googleapis.com/auth/classroom.coursework.students",
      "https://www.googleapis.com/auth/classroom.coursework.students.readonly",
      "https://www.googleapis.com/auth/drive.readonly",
      "https://www.googleapis.com/auth/classroom.courses",
      "https://www.googleapis.com/auth/classroom.rosters.readonly"};
}

void Server::handleRequest() {}

std::string Server::getAuthUrl() {
  std::string authUrl = config_.authUrl;
  authUrl += "scope=" + config_.scope + '&';
  authUrl += "response_type=" + config_.responseType + '&';
  authUrl += "redirect_uri=" + config_.redirectUrl + '&';
  authUrl += "client_id=" + config_.clientId;
  return authUrl;
}

void Server::authenticate() {}

void Server::run_server() {
  
}

Server::~Server() {

}