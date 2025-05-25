#include "rpc_server.hpp"
#include "paxos_message.hpp"
#include <iostream>

std::string message_type_to_string(MessageType t) {
    switch (t) {
        case MessageType::PREPARE: return "PREPARE";
        case MessageType::PROMISE: return "PROMISE";
        case MessageType::ACCEPT: return "ACCEPT";
        case MessageType::ACCEPTED: return "ACCEPTED";
        default: return "UNKNOWN";
    }
}

int main() {
    RPCServer server;
    server.start(8000, [](const PaxosMessage& msg) {
        std::cout << "Received message:\n";
        std::cout << "timestamp: "<<msg.timestamp<<"\n";
        std::cout << "type: " << message_type_to_string(msg.type) << "\n";
        std::cout << "key: " << msg.key << ", Value: " << msg.value << "\n";
        std::cout << "proposal_number: " << msg.proposal_number << ", From: " << msg.proposer_id << "\n";
    });
    return 0;
}
