#include "paxos_message.hpp"
#include <functional>

class RPCServer{
    public:
        void start(int port, std::function<void(PaxosMessage)> handler);
};