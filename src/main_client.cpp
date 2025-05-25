#include "rpc_client.hpp"
#include "paxos_message.hpp"
#include <iostream>
#include "timestamp.hpp"

int main() {
    PaxosMessage msg;
    msg.type = MessageType::PREPARE;
    msg.key = "x";
    msg.value = "42";
    msg.proposal_number = 1;
    msg.proposer_id = "node1";
    msg.timestamp = Timestamp::now_ms();

    std::string serialized = msg.serialize();
    std::cout << "[CLIENT] Sending JSON:\n" << serialized << "\n";

    RPCClient client;
    client.send("127.0.0.1", 8000, msg);

    return 0;
}
