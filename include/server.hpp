#pragma once

#include "kivitree.hpp"
#include <string>

class KiviTreeServer{
    private:
        int port;
        KiviTree store;
        int server_fd;

        void handle_client(int cliend_fd);
    
    public:
        explicit KiviTreeServer(int port=9000, size_t capacity = 100);
        void run();
};
