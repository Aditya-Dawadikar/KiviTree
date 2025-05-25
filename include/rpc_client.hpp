#include "paxos_message.hpp"

class RPCClient{
    public:
        bool send(const std::string& node_addr, int port, const PaxosMessage& msg);
};