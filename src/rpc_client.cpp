#include "rpc_client.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>   // ← for inet_pton
#include <unistd.h>      // ← for close()
#include <cstring>       // ← for memset

bool RPCClient::send(const std::string& node_addr, int port, const Message& msg){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return false;

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, node_addr.c_str(), &serv_addr.sin_addr);

    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        return false;
    }

    std::string data = msg.serialize();
    ::send(sockfd, data.c_str(), data.size(), 0);

    close(sockfd);
    return true;
}
