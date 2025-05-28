#pragma once
#include "kivitree_utils/message.hpp"

class RPCClient{
    public:
        bool send(const std::string& node_addr, int port, const Message& msg);
};