#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <set>
#include <mutex>
#include <thread>

namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

std::mutex g_mutex;
std::set<std::shared_ptr<websocket::stream<tcp::socket>>> g_clients;

// Typ MIME
std::string mime_type(const std::string& path) {
    if(path.size() >= 5 && path.substr(path.size()-5) == ".html") return "text/html";
    if(path.size() >= 3 && path.substr(path.size()-3) == ".js") return "application/javascript";
    if(path.size() >= 4 && path.substr(path.size()-4) == ".css") return "text/css";
    return "text/plain";
}

// Wczytanie pliku
std::string load_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if(!file) return "File not found";
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
}

// Obsługa jednej sesji
void handle_session(tcp::socket socket) {
    try {
        boost::beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        if(websocket::is_upgrade(req)) {
            auto ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
            ws->accept(req);

            {
                std::lock_guard<std::mutex> lock(g_mutex);
                g_clients.insert(ws);
            }

            while(true) {
                boost::beast::flat_buffer msg_buffer;
                ws->read(msg_buffer);
                std::string message = boost::beast::buffers_to_string(msg_buffer.data());

                // Broadcast do wszystkich innych klientów
                std::lock_guard<std::mutex> lock(g_mutex);
                for(auto& client : g_clients) {
                    if(client != ws && client->next_layer().is_open())
                        client->write(asio::buffer(message));
                }
            }
        } else {
            // HTTP GET – serwujemy pliki z frontend/
            std::string target = req.target().to_string();
            if(target == "/") target = "/index.html";

            std::string body = load_file("frontend" + target);

            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::server, "C++ Chat Server");
            res.set(http::field::content_type, mime_type(target));
            res.body() = body;
            res.prepare_payload();
            http::write(socket, res);
        }

    } catch(std::exception& e) {
        std::lock_guard<std::mutex> lock(g_mutex);
        for(auto it = g_clients.begin(); it != g_clients.end(); ) {
            if(it->get()->next_layer().is_open())
                ++it;
            else
                it = g_clients.erase(it);
        }
    }
}

int main() {
    try {
        asio::io_context ioc{1};

        const char* port_env = std::getenv("PORT");
        unsigned short port = port_env ? static_cast<unsigned short>(std::stoi(port_env)) : 9001;

        tcp::acceptor acceptor{ioc, {tcp::v4(), port}};
        std::cout << "Server running on port " << port << "\n";

        while(true) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);
            std::thread(handle_session, std::move(socket)).detach();
        }

    } catch(std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
    }
}
