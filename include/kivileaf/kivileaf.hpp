#pragma once
#include "kivitree_paxos/paxos_node.hpp"
#include <string>
#include "kivitree_core/kivitree.hpp"
#include <thread>
#include <set>

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

        bool send_self_promotion_notif = false;
        long long int self_promotion_ack_count = 0;
        std::set<long long int> promotion_ack_set;

        KiviTree kivi;
        

        // KiviLeaf(std::string node_ip, int node_port, long long int node_id, int leaf_role, bool is_proposer, int current_leader_id);
        KiviLeaf(std::string node_ip, int node_port, long long int node_id, int leaf_role, bool is_proposer);
        PaxosNodeDescriptor get_current_leader();
        void pull_leader_data();
        void push_leader_data(PaxosNodeDescriptor requester_node);
        void push_to_followers(std::string key, std::string value);
        void check_leader_health();
        // void set_local_cluster_leader();
        void try_self_promote();
        void on_startup_leader_detection (){
            PaxosNodeDescriptor curr_leader = get_current_leader();
            if (curr_leader.node_id == -1){
                this->try_self_promote(); 
            }else{
                current_leader = curr_leader;
            }
        }
        void on_leader_failure_detected() override {
            this->try_self_promote();
        }
        void broadcast_self_promotion();
        void run(int port);
        void initiate_message_server();
        bool has_majority(int votes);

        virtual ~KiviLeaf() = default;
};