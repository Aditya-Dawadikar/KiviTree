#include "paxos_node.hpp"
#include "message.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <node_id> <ip> <port> <role>\n";
        return 1;
    }

    std::string node_id = argv[1];
    std::string ip = argv[2];
    int port = std::stoi(argv[3]);
    std::string role = argv[4];
    bool is_proposer = (role == "proposer");

    PaxosNode node(node_id, ip, port, is_proposer);

    // Predefined cluster
    std::vector<PaxosNode::PaxosNodeSharableDescriptor> cluster = {
        {"P1", "127.0.0.1", 5000},
        {"A1", "127.0.0.1", 5001},
        {"A2", "127.0.0.1", 5002}
    };

    node.join_cluster(cluster);

    node.start(port, [](const Message& msg) {
        std::cout << "[HANDLER] Received message of type " << static_cast<int>(msg.get_message_type()) << "\n";
    });

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return 0;
}
