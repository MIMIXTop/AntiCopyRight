#include <boost/asio/io_context.hpp>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <vector>
#include <memory>
#include <boost/asio/ip/tcp.hpp>

namespace asio = boost::asio;
// namespace beast = boost::beast;
// namespace http = beast::http;
// namespace json = boost::json;
using tcp = asio::ip::tcp;

class Server {
public:    
    Server(std::vector<std::string> scopes = getDefaultScope(), int port = 8080);
    ~Server();
    void handleRequest();
    std::string getAuthUrl();
    void authenticate();
private:
    struct {
        std::string authUrl;
        std::string responseType;
        std::string redirectUrl;
        std::string clientId;
        std::string scope;
    } config_;

    void load_config(std::vector<std::string> scopes);
    static std::vector<std::string> getDefaultScope();
    void run_server();
    
    int port_;

};