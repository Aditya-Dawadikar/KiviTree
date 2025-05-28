#include "kivitree_server/server.hpp"

int main(int argc, char* argv[]) {
    int port = 9000;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    KiviTreeServer server(port);
    server.run();

    return 0;
}
