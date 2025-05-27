#include "rpc_server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>   // for htons, inet_ntoa
#include <cstring>       // for memset
#include <iostream>      // optional logging
#include "message.hpp"
#include "message_factory.hpp"

void RPCServer::start(int port, std::function<void(const Message&)> handler) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    std::cout<<"Paxos Server listening at port:"<<port<<"\n";

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        char buffer[4096] = {0};
        read(client_fd, buffer, 4096);
        
        std::string json_str(buffer);

        try {
            auto msg_ptr = MessageFactory::from_json(json_str);
            handler(*msg_ptr);  // calls handler with the correct derived class
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Message parse failed: " << e.what() << "\n";
        }

        close(client_fd);
    }
}
