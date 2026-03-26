#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <set>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

// --- WebSocket session ---
class WsSession : public std::enable_shared_from_this<WsSession> {
    websocket::stream<tcp::socket> ws_;
    std::shared_ptr<std::set<std::shared_ptr<WsSession>>> sessions_;
    beast::flat_buffer buffer_;

public:
    WsSession(tcp::socket socket,
              std::shared_ptr<std::set<std::shared_ptr<WsSession>>> sessions)
        : ws_(std::move(socket)), sessions_(sessions) {}

    void start() {
        ws_.async_accept([self=shared_from_this()](beast::error_code ec){
            if(!ec){
                self->sessions_->insert(self);
                self->do_read();
            }
        });
    }

    void do_read() {
        ws_.async_read(buffer_, [self=shared_from_this()](beast::error_code ec, std::size_t){
            if(ec){
                self->sessions_->erase(self);
                return;
            }
            std::string msg = beast::buffers_to_string(self->buffer_.data());

            // Broadcast to all sessions
            for(auto& s : *self->sessions_){
                s->ws_.text(true);
                s->ws_.async_write(asio::buffer(msg),
                    [](beast::error_code, std::size_t){});
            }
            self->buffer_.consume(self->buffer_.size());
            self->do_read();
        });
    }
};

// --- Helper: ends_with for C++17 ---
bool ends_with(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// --- HTTP session for static files ---
void handle_http(tcp::socket socket) {
    try {
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        std::string target = req.target().to_string();
        if(target == "/") target = "/index.html";

        std::string path = "frontend" + target;
        if(!boost::filesystem::exists(path)){
            http::response<http::string_body> res{http::status::not_found, 11};
            res.set(http::field::content_type, "text/plain");
            res.body() = "Not Found";
            res.prepare_payload();
            http::write(socket, res);
            return;
        }

        std::ifstream file(path, std::ios::binary);
        std::string body((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());

        http::response<http::string_body> res{http::status::ok, 11};
        if(ends_with(path,".html")) res.set(http::field::content_type, "text/html");
        else if(ends_with(path,".js")) res.set(http::field::content_type, "application/javascript");
        else if(ends_with(path,".css")) res.set(http::field::content_type, "text/css");
        else res.set(http::field::content_type, "text/plain");

        res.body() = body;
        res.prepare_payload();
        http::write(socket, res);
    } catch(...) {}
}

// --- Accept incoming connections ---
void accept_connections(tcp::acceptor& acceptor,
                        std::shared_ptr<std::set<std::shared_ptr<WsSession>>> sessions,
                        asio::io_context& ioc) {
    acceptor.async_accept([&](beast::error_code ec, tcp::socket socket){
        if(!ec){
            // Jeśli pierwszy bajt to 'G', traktujemy jako HTTP GET
            socket.async_wait(tcp::socket::wait_read, [&](beast::error_code ec2){
                if(!ec2){
                    char peek[1];
                    beast::error_code ec3;
                    size_t n = socket.read_some(asio::buffer(peek), ec3);
                    if(n>0 && peek[0]=='G'){
                        std::thread(handle_http, std::move(socket)).detach();
                    } else {
                        std::make_shared<WsSession>(std::move(socket), sessions)->start();
                    }
                }
            });
        }
        accept_connections(acceptor, sessions, ioc);
    });
}

int main() {
    try {
        asio::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {tcp::v4(), 9001}};
        auto sessions = std::make_shared<std::set<std::shared_ptr<WsSession>>>();

        std::cout << "Server running on port 9001\n";

        accept_connections(acceptor, sessions, ioc);
        ioc.run();

    } catch(std::exception& e){
        std::cerr << "Error: " << e.what() << "\n";
    }
}
