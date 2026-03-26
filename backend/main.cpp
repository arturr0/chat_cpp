#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <thread>
#include <set>
#include <mutex>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// 🔥 globalna lista klientów
std::set<websocket::stream<tcp::socket>*> clients;
std::mutex clients_mutex;

// 🔧 czytanie pliku
std::string read_file(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 🔥 broadcast
void broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto* client : clients) {
        try {
            client->text(true);
            client->write(net::buffer(message));
        } catch (...) {}
    }
}

void handle_client(tcp::socket socket) {
    try {
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        // 🔥 WEBSOCKET
        if (websocket::is_upgrade(req)) {

            auto ws = new websocket::stream<tcp::socket>(std::move(socket));
            ws->accept(req);

            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.insert(ws);
            }

            std::cout << "Client connected\n";

            while (true) {
                beast::flat_buffer buffer;
                ws->read(buffer);

                std::string msg = beast::buffers_to_string(buffer.data());
                std::cout << "Msg: " << msg << "\n";

                broadcast(msg);
            }

        } else {
            // 🌐 STATIC FILES
            std::string target = std::string(req.target());

            if (target == "/") target = "/index.html";

            std::string path = "." + target;
            std::string body = read_file(path);

            if (body.empty()) {
                http::response<http::string_body> res{
                    http::status::not_found, req.version()
                };
                res.body() = "404 Not Found";
                res.prepare_payload();
                http::write(socket, res);
                return;
            }

            http::response<http::string_body> res{
                http::status::ok, req.version()
            };

            // 🔥 content-type
            if (target.ends_with(".html")) res.set(http::field::content_type, "text/html");
            else if (target.ends_with(".js")) res.set(http::field::content_type, "application/javascript");
            else if (target.ends_with(".css")) res.set(http::field::content_type, "text/css");

            res.body() = body;
            res.prepare_payload();

            http::write(socket, res);
        }

    } catch (std::exception const& e) {
        std::cout << "Client disconnected\n";
    }
}

int main() {
    try {
        int port = 9001;
        if (const char* env_p = std::getenv("PORT")) {
            port = std::stoi(env_p);
        }

        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {tcp::v4(), (unsigned short)port}};

        std::cout << "Server running on port " << port << "\n";

        while (true) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            std::thread{handle_client, std::move(socket)}.detach();
        }

    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
