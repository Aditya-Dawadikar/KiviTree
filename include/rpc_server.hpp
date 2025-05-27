#include <functional>
#include "message.hpp"

class RPCServer{
    public:
        static void start(int port, std::function<void(const Message&)> handler);
};