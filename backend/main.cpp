#include <uwebsockets/App.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

std::string readFile(const std::string& path) {
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

int main() {
    uWS::App().get("/*", [](auto *res, auto *req) {
        std::string url = req->getUrl().toString();
        if (url == "/") url = "/index.html";

        std::string path = "." + url; // teraz frontend jest w tym samym folderze
        std::ifstream f(path);
        if (!f.is_open()) {
            res->writeStatus("404 Not Found")->end("File not found");
            return;
        }

        std::string content = readFile(path);
        // typ content-type na podstawie rozszerzenia
        if (url.size() >= 5 && url.substr(url.size()-5) == ".html") res->writeHeader("Content-Type", "text/html");
        else if (url.size() >= 3 && url.substr(url.size()-3) == ".js") res->writeHeader("Content-Type", "application/javascript");
        else if (url.size() >= 4 && url.substr(url.size()-4) == ".css") res->writeHeader("Content-Type", "text/css");
        else res->writeHeader("Content-Type", "text/plain");

        res->end(content);
    })
    .ws("/*", {
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
    .listen(9001, [](auto *token){
        if (token) {
            std::cout << "Server running on port 9001\n";
        }
    })
    .run();
}
