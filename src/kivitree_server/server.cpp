#include "kivitree_server/server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <chrono>
#include <ctime>
#include <thread>

// Helper function for timestamp
std::string current_time() {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&now));
    return buf;
}

KiviTreeServer::KiviTreeServer(int port, size_t capacity)
    : port(port), store(capacity), server_fd(-1) {}

void KiviTreeServer::run() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return;
    }

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        return;
    }

    listen(server_fd, 5);
    std::cout << "[KiViTree] Listening on port " << port << "...\n";

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd >= 0) {
            // handle_client(client_fd);
            // close(client_fd);
            std::thread(&KiviTreeServer::handle_client, this, client_fd).detach();
        }
    }
}

void KiviTreeServer::handle_client(int client_fd) {
    char buffer[1024];

    while (true) {
        ssize_t bytes = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        std::istringstream iss(buffer);
        std::string cmd, key, value;
        iss >> cmd >> key;
        std::ostringstream response;

        std::string action_summary;

        if (cmd == "GET") {
            auto result = store.get(key);
            response << (result ? *result : "NOT_FOUND") << "\n";
            action_summary = "GET " + key;
        } else if (cmd == "PUT") {
            iss >> std::ws;
            std::getline(iss, value);
            store.put(key, value);
            response << "OK\n";
            action_summary = "PUT " + key + " = " + value;
        } else if (cmd == "DEL") {
            store.remove(key);
            response << "OK\n";
            action_summary = "DEL " + key;
        } else {
            response << "ERR_UNKNOWN_COMMAND\n";
            action_summary = "UNKNOWN COMMAND";
        }

        send(client_fd, response.str().c_str(), response.str().length(), 0);

        // Print log
        std::cout << "[" << current_time() << "] fd=" << client_fd
                  << " | " << action_summary
                  << " -> " << response.str();
    }
}
