#pragma once
#include "kivitree_paxos/paxos_node.hpp"
#include <string>
#include "kivitree_core/kivitree.hpp"
#include <thread>

enum KiviLeafRole{
    LOCAL_LEADER,
    PROMOTION_READY_REPLICA
};

class KiviLeaf:public PaxosNode{
    public:
        int leaf_role;
        int current_leader_id;
        PaxosNodeDescriptor current_leader;
        long long int last_sync_time;

        KiviTree kivi;
        

        KiviLeaf(std::string node_ip, int node_port, long long int node_id, int leaf_role, bool is_proposer, int current_leader_id);
        PaxosNodeDescriptor get_current_leader();
        void pull_leader_data();
        void push_leader_data(PaxosNodeDescriptor requester_node);
        void push_to_followers(std::string key, std::string value);
        void check_leader_health();
        void get_local_cluster_leader();
        void try_self_promote();
        void on_leader_failure_detected() override {
            this->try_self_promote();
        }
        void broadcast_self_promotion();
        void run(int port);
        void initiate_message_server();

        virtual ~KiviLeaf() = default;
};