#include "kivileaf/kivileaf.hpp"
#include "kivileaf/local_sync_request_message.hpp"
#include "kivileaf/local_sync_response_message.hpp"
#include "kivileaf/local_sync_push_message.hpp"
#include "kivitree_utils/timestamp.hpp"
#include "kivitree_rpc/rpc_server.hpp"
#include "kivileaf/kivileaf_rest_controller.hpp"
#include <unordered_map>
#include <future>
#include "kivileaf/local_sync_self_promotion.hpp"
#include "kivileaf/local_sync_self_promotion_ack.hpp"

KiviLeaf::KiviLeaf(std::string node_ip,
                    int node_port,
                    long long int node_id,
                    int leaf_role,
                    bool is_proposer)
                    :leaf_role(leaf_role), PaxosNode(node_id, node_ip, node_port, is_proposer){
    this->current_leader_id = -1;
    // TODO:
    // dont declare yourself the leader
    // interpret leadership status from the cluster
}

// void KiviLeaf::set_local_cluster_leader(){
//     // TODO:
//     // request all nodes to check who is the leader
//     // if no leader found, become the leader
// }

KiviLeaf::PaxosNodeDescriptor KiviLeaf::get_current_leader(){
    if (current_leader_id == -1){
        // must elect new leader
        return PaxosNodeDescriptor{-1, "", -1, -1, false};
    }else{
        PaxosNodeDescriptor curr_lead;
        for(const auto& node: local_cluster_nodes){
            if(node.node_id == current_leader_id){
                curr_lead.node_id = node.node_id;
                curr_lead.node_ip = node.node_ip;
                curr_lead.node_port = node.node_port;
                curr_lead.last_seen = node.last_seen;
                curr_lead.is_alive = node.is_alive;

                return curr_lead;
            }
        }

        throw std::invalid_argument("Current Leader: "+std::to_string(current_leader_id)+" not found\n");
    }
}
        
void KiviLeaf::pull_leader_data(){
    std::cout<<"[SYNC] Sending Local Sync Request\n";
    LocalSyncRequestMessage msg = LocalSyncRequestMessage(this->node_id, this->last_sync_time);
    this->send_message(current_leader.node_ip, current_leader.node_port, msg);
}

void KiviLeaf::push_leader_data(PaxosNodeDescriptor requester_node){

    std::cout<<"[SYNC] Sending Local Sync Response\n";

    std::unordered_map<std::string, std::string> batch_res = this->kivi.get_kv_batch();

    LocalSyncPayload payload;
    payload.load_from_map(batch_res);

    LocalSyncResponseMessage msg = LocalSyncResponseMessage(node_id,
                                                            requester_node.node_id,
                                                            requester_node.last_sync,
                                                            Timestamp::now_ms(),
                                                            payload);
    
    this->send_message(requester_node.node_ip, requester_node.node_port, msg);
}

void KiviLeaf::push_to_followers(std::string key, std::string value){
    std::vector<std::future<void>> tasks;
    LocalSyncPushMessage msg = LocalSyncPushMessage(node_id, Timestamp::now_ms(), key, value);
    for(const auto& follower: local_cluster_nodes){
        if(follower.node_id != node_id && follower.is_alive){
            tasks.push_back(std::async(std::launch::async, [this, follower, msg](){
                std::cout<<"[PUSHED] to "<<follower.node_id<<"\n";
                this->send_message(follower.node_ip, follower.node_port, msg);
            }));
        }
    }

    // Wait for all sends to finish
    for (auto& task : tasks) {
        task.get();
    }
}

void KiviLeaf::broadcast_self_promotion(){
    std::vector<std::future<void>> tasks;
    LocalSyncSelfPromotionMessage msg = LocalSyncSelfPromotionMessage(node_id, Timestamp::now_ms());
    for(const auto& follower: local_cluster_nodes){
        if(follower.node_id != node_id && follower.is_alive){
            tasks.push_back(std::async(std::launch::async, [this, follower, msg](){
                std::cout<<"[PROMOTE] Broadcasting self promotion to "<<follower.node_id<<"\n";
                this->send_message(follower.node_ip, follower.node_port, msg);
            }));
        }
    }

    // Wait for all sends to finish
    for (auto& task : tasks) {
        task.get();
    }
}

bool KiviLeaf::has_majority(int votes){


    int total_live_node_count = 0;
    for(const auto& node: local_cluster_nodes){
        if(node.is_alive){
            total_live_node_count += 1;
        }
    }

    long long int ack_count = promotion_ack_set.size();

    std::cout<<"Total Live Nodes: "<< total_live_node_count<<" , Total Votes: "<< ack_count<<"\n";

    if (ack_count >= std::floor(total_live_node_count/2) + 1){
        return true;
    }

    return false;
}

void KiviLeaf::try_self_promote(){
    if(leaf_role != PROMOTION_READY_REPLICA) return;

    try{
        auto leader = get_current_leader();
        
        for(const auto& node:local_cluster_nodes){
            if (node.node_id == leader.node_id){
                if(node.is_alive){return;}  // leader is still active
            }
        }

        std::cout<<"[LOCAL] Detected dead leader: "<< leader.node_id<<"\n";

        long long lowest_id = node_id;
        for(const auto& node: local_cluster_nodes){
            if(node.is_alive && node.node_id < lowest_id){
                lowest_id = node.node_id;
            }
        }

        if (lowest_id == node_id){
            // can promote self as leader
            std::cout << "[PROMOTE] I (" << node_id << ") am the lowest ID replica. Promoting myself to leader.\n";
            current_leader_id = node_id;
            leaf_role = LOCAL_LEADER;

            self_promotion_ack_count = 0;
            send_self_promotion_notif = true;

            // broadcast self election
            broadcast_self_promotion();

            // launch background broadcaster
            std::thread([this]() {
                while (true) {
                    if(this->has_majority(this->self_promotion_ack_count) && send_self_promotion_notif==true){
                        std::cout<<"[PROMOTE] Received Self Promotion Acknowledgement from Majority nodes\n";
                        send_self_promotion_notif = false;
                        promotion_ack_set.clear();
                        break;
                    }else{
                        std::cout << "[PROMOTE] Re-broadcasting self promotion. Waiting for acknowledgement...\n";
                        this->broadcast_self_promotion();
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            }).detach();
        } else {
            std::cout << "[LOCAL] Node (" << lowest_id << ") is eligible to become leader. Holding off.\n";
        }

    }catch (const std::exception& e) {
        std::cerr << "[LOCAL] Error determining current leader: " << e.what() << "\n";
    }
}

void KiviLeaf::initiate_message_server(){
    std::thread([this]() {
        RPCServer::start(this->node_port, [this](std::unique_ptr<Message> msg) {
            switch(msg->get_message_type()) {
                case MessageType::LOCAL_SYNC_PUSH: {
                    if(auto req = dynamic_cast<const LocalSyncPushMessage*>(msg.get())){
                        std::cout << "[LEAF] Received push msg from " << req->leader_node_id << "\n";
                        this->kivi.put(req->key, req->value);
                        this->last_sync_time = req->latest_sync_timestamp;
                    }else {
                        std::cerr << "[ERROR] Invalid LOCAL_SYNC_PUSH message\n";
                    }
                    break;
                }
                case MessageType::LOCAL_SYNC_REQUEST: {
                    if (auto req = dynamic_cast<const LocalSyncRequestMessage*>(msg.get())) {
                        std::cout << "[SYNC] Received sync request from " << req->follower_node_id << "\n";
                        
                        // find the follower in local cluster members
                        for(const auto& local_node: local_cluster_nodes){
                            if (local_node.node_id == req->follower_node_id){
                                // node found
                                PaxosNodeDescriptor requester;
                                requester.node_id = req->follower_node_id;
                                requester.node_ip = local_node.node_ip;
                                requester.node_port = local_node.node_port;
                                requester.last_seen = local_node.last_seen;
                                requester.last_sync = local_node.last_sync;

                                this->push_leader_data(requester);
                                return;
                            }
                        }
                        std::cerr << "[ERROR] Invalid LOCAL_SYNC_REQUEST message, Requester node not found\n";
                        
                    } else {
                        std::cerr << "[ERROR] Invalid LOCAL_SYNC_REQUEST message\n";
                    }
                    break;
                }
                case MessageType::LOCAL_SYNC_RESPONSE:{
                    if (auto res = dynamic_cast<const LocalSyncResponseMessage*>(msg.get())) {
                        std::cout << "[SYNC] Received sync response from " << res->leader_node_id<< "\n";
                        this->kivi.set_kv_batch(res->payload.get_payload());
                        this->last_sync_time = res->latest_sync_timestamp;
                    } else {
                        std::cerr << "[ERROR] Invalid LOCAL_SYNC_RESPONSE message\n";
                    }
                    break;
                }
                case MessageType::LOCAL_SYNC_SELF_PROMOTION:{
                    if (auto res = dynamic_cast<const LocalSyncSelfPromotionMessage*>(msg.get())) {
                        
                        if (res->leader_node_id == this->node_id) {
                            std::cout << "[PROMOTE] Ignoring self promotion message from self\n";
                            break;
                        }
                        
                        std::cout << "[PROMOTE] Received promotion notification from " << res->leader_node_id<< "\n";

                        this->current_leader_id = res->leader_node_id;
                        for(const auto& node: local_cluster_nodes){
                            if(node.node_id == res->leader_node_id){
                                current_leader.node_id = node.node_id;
                                current_leader.node_ip = node.node_ip;
                                current_leader.node_port = node.node_port;
                                current_leader.is_alive = node.is_alive;
                                current_leader.last_seen = Timestamp::now_ms();
                                current_leader.last_sync = -1;

                                // ✅ Send ACK here using correct destination
                                LocalSyncSelfPromotionAckMessage ack = LocalSyncSelfPromotionAckMessage(
                                    node.node_id,               // leader_id
                                    res->promotion_timestamp,  // same timestamp
                                    this->node_id,             // self id
                                    Timestamp::now_ms()
                                );
                                std::cout<<"[PROMOTE] Sending Self Promotion Acknowledgement to: "<<node.node_id<<" "<<node.node_ip<<":"<<node.node_port<<"\n";
                                this->send_message(node.node_ip, node.node_port, ack);  // ✅ reliable destination
                                break; // exit loop

                            }
                        }

                        // pull data from leader for sync
                        this->pull_leader_data();

                    } else {
                        std::cerr << "[ERROR] Invalid LOCAL_SYNC_SELF_PROMOTION message\n";
                    }
                    break;
                }
                case MessageType::LOCAL_SYNC_SELF_PROMOTION_ACK:{
                    if (auto res = dynamic_cast<const LocalSyncSelfPromotionAckMessage*>(msg.get())) {
                        std::cout << "[PROMOTE] Received promotion acknowledgement notification from " << res->follower_node_id<< "\n";
                        
                        if(promotion_ack_set.find(res->follower_node_id) != promotion_ack_set.end()){
                            // already received ack from node
                            std::cout<<"[PROMOTE] Vote already counted for node "<<res->follower_node_id<<"\n";
                        }else{
                            self_promotion_ack_count += 1;
                            promotion_ack_set.insert(res->follower_node_id);
                        }
                    } else {
                        std::cerr << "[ERROR] Invalid LOCAL_SYNC_SELF_PROMOTION_ACK message\n";
                    }
                    break;
                }
                default: {
                    // std::cout << "[LEAF] Forwarding to Paxos handler\n";
                    this->handle_paxos_message(std::move(msg));
                    break;
                }
            }
        });
    }).detach();
}

void KiviLeaf::run(int port){

    this->node_port = port;
    
    std::cout << "[LEAF] Starting KiviLeaf node " << node_id << " on port " << node_port << "\n";

    // thread to send heart beats
    this->initiate_heartbeat();

    // thread to detect failing nodes
    this->initiate_heartbeat_failure_detection();

    // thread to accept messages from cluster nodes
    this->initiate_message_server();

    // elect local leader
    this->on_startup_leader_detection();

    // fetch data from leader on boot
    try {
        this->current_leader = this->get_current_leader();
        if (this->node_id != this->current_leader.node_id){
            this->pull_leader_data();
        }
    } catch (const std::exception& e) {
        std::cerr << "[WARN] Failed to fetch leader on boot: " << e.what() << "\n";
    }

    // Start REST API
    int rest_port = node_port + 1000;
    auto* rest = new KiviLeafRestController(this);
    rest->start(rest_port);

}