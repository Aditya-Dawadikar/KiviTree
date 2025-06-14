#include "kivitree_paxos/paxos_node.hpp"
#include "kivitree_utils/message.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "kivileaf/kivileaf.hpp"

int main(int argc, char* argv[]) {
    if (argc < 7) {
        std::cerr << "Usage: " << argv[0] << " <node_id> <ip> <port> <leaf_role> <is_proposer> <leader_id>\n";
        return 1;
    }

    /*
    ./kivitree 10000 127.0.0.1 5000 0 1 10000
    ./kivitree 10001 127.0.0.1 5001 1 0 10000
    ./kivitree 10002 127.0.0.1 5002 1 0 10000
    */

    long long int node_id = std::stoi(argv[1]);
    std::string node_ip = argv[2];
    int node_port = std::stoi(argv[3]);
    int leaf_role = std::stoi(argv[4]);
    bool is_proposer = std::stoi(argv[5])==1?true:false;
    int leader_id = std::stoi(argv[6]);

    // PaxosNode node(node_id, ip, port, is_proposer);

    KiviLeaf node = KiviLeaf(node_ip,
                                node_port,
                                node_id,
                                leaf_role,
                                is_proposer,
                                leader_id);

    // Predefined cluster
    std::vector<PaxosNode::PaxosNodeSharableDescriptor> cluster = {
        {10000, "127.0.0.1", 5000},
        {10001, "127.0.0.1", 5001},
        {10002, "127.0.0.1", 5002}
    };

    node.register_cluster_nodes(cluster);

    node.set_local_cluster_leader(10000);

    node.run(node_port);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return 0;
}
