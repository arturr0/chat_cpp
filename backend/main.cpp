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

std::set<websocket::stream<tcp::socket>*> clients;
std::mutex clients_mutex;

bool ends_with(const std::string& str, const std::string& suffix) {
    if (str.size() < suffix.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

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
                broadcast(msg);
            }

        } else {
            // 🔥 Serwujemy frontend z folderu /frontend
            std::string target = std::string(req.target());
            if (target == "/") target = "/index.html";

            std::string path = "./frontend" + target;
            std::string body = read_file(path);

            http::response<http::string_body> res{
                body.empty() ? http::status::not_found : http::status::ok,
                req.version()
            };

            if (body.empty()) {
                res.body() = "404 Not Found";
            } else {
                if (ends_with(target, ".html")) res.set(http::field::content_type, "text/html");
                else if (ends_with(target, ".js")) res.set(http::field::content_type, "application/javascript");
                else if (ends_with(target, ".css")) res.set(http::field::content_type, "text/css");
                res.body() = body;
            }
            res.prepare_payload();
            http::write(socket, res);
        }

    } catch (...) {
        std::cout << "Client disconnected\n";
    }
}

int main() {
    try {
        int port = 9001;
        if (const char* env_p = std::getenv("PORT")) port = std::stoi(env_p);

        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {tcp::v4(), static_cast<unsigned short>(port)}};

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
