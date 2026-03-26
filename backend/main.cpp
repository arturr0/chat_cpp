#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <thread>
#include <cstdlib>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

void handle_client(tcp::socket socket) {
    try {
        websocket::stream<tcp::socket> ws(std::move(socket));
        ws.accept();

        std::cout << "Client connected\n";

        while (true) {
            beast::flat_buffer buffer;
            ws.read(buffer);

            std::string msg = beast::buffers_to_string(buffer.data());
            std::cout << "Received: " << msg << "\n";

            ws.text(ws.got_text());
            ws.write(net::buffer(msg));
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

        std::cout << "Server listening on port " << port << "\n";

        while (true) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            std::thread{handle_client, std::move(socket)}.detach();
        }

    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
