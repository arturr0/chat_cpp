#include <uwebsockets/App.h>
#include <iostream>
#include <string>

int main() {
    uWS::App().ws("/*", {
        .open = [](auto* ws) {
            std::cout << "Client connected\n";
        },

        .message = [](auto* ws, std::string_view message, uWS::OpCode opCode) {
            ws->publish("chat", message, opCode);
        },

        .close = [](auto* ws, int code, std::string_view msg) {
            std::cout << "Client disconnected\n";
        }
    })
    .listen(9001, [](auto* token) {
        if (token) {
            std::cout << "Server listening on port 9001\n";
        }
    })
    .run();
}