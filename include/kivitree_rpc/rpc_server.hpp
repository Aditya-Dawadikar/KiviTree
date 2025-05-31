#include <functional>
#include "kivitree_utils/message.hpp"
#include <memory>

class RPCServer{
    public:
        static void start(int port, std::function<void(std::unique_ptr<Message>)> handler);
};