#include "kivitree_paxos/paxos_node.hpp"
#include "kivitree_rpc/rpc_server.hpp"
#include "kivitree_rpc/rpc_client.hpp"
#include "kivitree_utils/timestamp.hpp"
#include <thread>
#include <iostream>
#include "kivitree_utils/message.hpp"
#include "future"
#include "kivitree_paxos/prepare_message.hpp"
#include "kivitree_paxos/promise_message.hpp"
#include "kivitree_paxos/accept_message.hpp"
#include "kivitree_paxos/accepted_message.hpp"
#include "kivitree_paxos/heartbeat_message.hpp"
#include <iomanip>
#include <sstream>

const int HEARTBEAT_SLEEP_TIME = 5; // 3 seconds
const int HEARTBEAT_SKIP_COUNT = 3; // accomodate 3 skipped heartbeats

PaxosNode::PaxosNode(long long int node_id, std::string node_ip, int node_port, bool is_proposer)
                    :node_id(node_id),node_ip(node_ip), node_port(node_port), is_proposer(is_proposer){}

void PaxosNode::log_cluster(){
    std::cout << "\n[CLUSTER NODES]\n";
    std::cout << std::left
              << std::setw(10) << "Node ID"
              << std::setw(18) << "IP"
              << std::setw(8)  << "Port"
              << std::setw(22) << "Last Seen"
              << std::setw(8)  << "Alive"
              << std::setw(22) << "Last Sync"
              << "\n";

    std::cout << std::string(80, '-') << "\n";

    std::cout << std::string(88, '-') << "\n";

    for (const auto& node : cluster_nodes) {
        time_t local_time = static_cast<time_t>(node.last_seen / 1000);

        std::ostringstream time_stream;
        time_stream << std::put_time(std::localtime(&local_time), "%Y-%m-%d %H:%M:%S");
        std::string formatted_time = time_stream.str();

        std::string last_sync_str = (node.last_sync == -1)
                                    ? "N/A"
                                    : std::to_string(node.last_sync);

        std::cout << std::left
                << std::setw(10) << node.node_id
                << std::setw(18) << node.node_ip
                << std::setw(8)  << node.node_port
                << std::setw(22) << formatted_time
                << std::setw(8)  << (node.is_alive ? "Yes" : "No")
                << std::setw(22) << last_sync_str
                << "\n";
    }
}


void PaxosNode::log_local_cluster() {
    std::cout << "\n[LOCAL CLUSTER NODES]\n";
    std::cout << std::left
              << std::setw(10) << "Node ID"
              << std::setw(18) << "IP"
              << std::setw(8)  << "Port"
              << std::setw(22) << "Last Seen"
              << std::setw(8)  << "Alive"
              << std::setw(22) << "Last Sync"
              << "\n";

    std::cout << std::string(88, '-') << "\n";

    for (const auto& node : local_cluster_nodes) {
        time_t local_time = static_cast<time_t>(node.last_seen / 1000);

        std::ostringstream time_stream;
        time_stream << std::put_time(std::localtime(&local_time), "%Y-%m-%d %H:%M:%S");
        std::string formatted_time = time_stream.str();

        std::string last_sync_str = (node.last_sync == -1)
                                    ? "N/A"
                                    : std::to_string(node.last_sync);

        std::cout << std::left
                << std::setw(10) << node.node_id
                << std::setw(18) << node.node_ip
                << std::setw(8)  << node.node_port
                << std::setw(22) << formatted_time
                << std::setw(8)  << (node.is_alive ? "Yes" : "No")
                << std::setw(22) << last_sync_str
                << "\n";
    }

}

void PaxosNode::iniate_heartbeat(){
    std::thread([this](){
        try{
            std::cout << "[THREAD] Heartbeat thread running\n";
            while(true){
                std::cout << "[ME] ("<<Timestamp::now_ms()<<") I am Alive\n";
                // log_local_cluster();

                HeartBeatMessage msg_cluster, msg_local;

                msg_cluster.node_id = this->node_id;
                msg_cluster.node_ip = this->node_ip;
                msg_cluster.node_port = this->node_port;
                msg_cluster.cluster_type = 1;
                msg_cluster.last_seen = Timestamp::now_ms();

                msg_local.node_id = this->node_id;
                msg_local.node_ip = this->node_ip;
                msg_local.node_port = this->node_port;
                msg_local.last_seen = 2;
                msg_local.last_seen = Timestamp::now_ms();

                // broadcast heartbeat to cluster nodes
                std::thread t1([this, msg_cluster](){
                    try{    
                        broadcast_heart_beat(msg_cluster, 1);
                    }catch(const std::exception& e){
                        std::cerr << "[ERROR] cluster_nodes broadcast failed: " << e.what() << "\n";
                    }
                });

                // broadcast heartbeat to read-only replicas
                std::thread t2([this, msg_local](){
                    try{      
                        broadcast_heart_beat(msg_local, 2);
                    }catch(const std::exception& e){
                        std::cerr << "[ERROR] cluster_nodes broadcast failed: " << e.what() << "\n";
                    }
                });

                t1.join();
                t2.join();

                std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_SLEEP_TIME));
            }
        } catch (const std::exception& e) {
            std::cerr << "[EXCEPTION] Heartbeat thread crashed: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "[EXCEPTION] Heartbeat thread crashed with unknown error.\n";
        }
    }).detach();

    std::cout << "[INFO] Heartbeat thread launched successfully\n";
}

void PaxosNode::initiate_heartbeat_failure_detection(){
    std::thread([this](){
        try{
            std::cout << "[THREAD] Heartbeat Failure detection thread running\n";
            while(true){
                long long now = Timestamp::now_ms();
                for(auto& node: cluster_nodes){
                    if(node.node_id == this->node_id) {
                        node.is_alive = true;
                        continue;
                    };

                    // std::cout<<"Last Seen: "<<node.node_id<<" "<<(now - node.last_seen)<<"\n";
                    if(now - node.last_seen > 1000*HEARTBEAT_SLEEP_TIME*HEARTBEAT_SKIP_COUNT && node.is_alive){
                        // mark node as dead
                        node.is_alive = false;
                        std::cout << "[DEAD] Node "<< node.node_id<<" marked as DEAD\n";
                    }
                }
                for(auto& node: local_cluster_nodes){
                    if(node.node_id == this->node_id) {
                        node.is_alive = true;
                        continue;
                    };

                    // std::cout<<"Last Seen: "<<node.node_id<<" "<<(now - node.last_seen)<<"\n";
                    if(now - node.last_seen > 1000*HEARTBEAT_SLEEP_TIME*HEARTBEAT_SKIP_COUNT && node.is_alive){
                        // mark node as dead
                        node.is_alive = false;
                        std::cout << "[DEAD] Node "<< node.node_id<<" marked as DEAD\n";
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_SLEEP_TIME));
            }
        }catch (const std::exception& e) {
            std::cerr << "[EXCEPTION] Heartbeat Failure Detection thread crashed: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "[EXCEPTION] Heartbeat Failure Detection thread crashed with unknown error.\n";
        }

    }).detach();

    std::cout << "[INFO] Heartbeat Detection thread launched successfully\n";
}

void PaxosNode::handle_paxos_message(std::unique_ptr<Message> msg){
    switch (msg->get_message_type()) {
            case MessageType::HEART_BEAT: {
                if (auto* hb_msg = dynamic_cast<const HeartBeatMessage*>(msg.get())) {
                    this->receive_heart_beat(*hb_msg);
                } else {
                    std::cerr << "[ERROR] Failed to cast to HeartBeatMessage\n";
                }
                break;
            }

            case MessageType::PREPARE: {
                if (auto prep = dynamic_cast<const PrepareMessage*>(msg.get())) {
                    // handler(*prep);
                } else {
                    std::cerr << "[ERROR] Failed to cast to PrepareMessage\n";
                }
                break;
            }

            case MessageType::PROMISE: {
                if (auto prom = dynamic_cast<const PromiseMessage*>(msg.get())) {
                    // handler(*prom);
                } else {
                    std::cerr << "[ERROR] Failed to cast to PromiseMessage\n";
                }
                break;
            }

            case MessageType::ACCEPT: {
                if (auto acc = dynamic_cast<const AcceptMessage*>(msg.get())) {
                    // handler(*acc);
                } else {
                    std::cerr << "[ERROR] Failed to cast to AcceptMessage\n";
                }
                break;
            }

            case MessageType::ACCEPTED: {
                if (auto accd = dynamic_cast<const AcceptedMessage*>(msg.get())) {
                    // handler(*accd);
                } else {
                    std::cerr << "[ERROR] Failed to cast to AcceptedMessage\n";
                }
                break;
            }

            default:
                std::cerr << "[ERROR] Unknown message type received\n";
        }
}

bool PaxosNode::is_node_alive(const long long int node_id){
    for(const auto& node:cluster_nodes){
        if (node.node_id == node_id){
            return node.is_alive;
        }
    }
    return false;   //unknown node
}

bool PaxosNode::update_last_seen(const HeartBeatMessage& msg){
    // std::cout<<"Incoming Heartbeat from: "<<msg.node_id<<std::endl;
    for(auto& node: cluster_nodes){
        if(node.node_id == msg.node_id){
            if(node.is_alive == false){
                std::cout<<"[ALIVE] Node "<< node.node_id<<" marked as ALIVE\n";
            }
            node.last_seen = msg.last_seen;
            node.is_alive = true;
            return true;
        }
    }
    for(auto& node: local_cluster_nodes){
        if(node.node_id == msg.node_id){
            if(node.is_alive == false){
                std::cout<<"[ALIVE] Node "<< node.node_id<<" marked as ALIVE\n";
            }
            node.last_seen = msg.last_seen;
            node.is_alive = true;
            return true;
        }
    }
    return false;
}

void PaxosNode::add_new_node(PaxosNodeSharableDescriptor new_node, int cluster_type){

    PaxosNodeDescriptor desc;
    desc.node_id = new_node.node_id;
    desc.node_ip = new_node.node_ip;
    desc.node_port = new_node.node_port;
    desc.last_seen = Timestamp::now_ms();
    desc.last_sync = -1;
    desc.is_alive = true;

    if (cluster_type == 1){
        // add to global cluster
        cluster_nodes.push_back(desc);
        std::cout<<"[INFO] New node added to cluster: "<<new_node.node_id <<"\n";
    }else{
        // add to local cluster
        local_cluster_nodes.push_back(desc);
        std::cout<<"[INFO] New node added to local cluster: "<<new_node.node_id <<"\n";
    }
}

void PaxosNode::remove_node(PaxosNodeSharableDescriptor node){
    cluster_nodes.erase(std::remove_if(cluster_nodes.begin(), cluster_nodes.end(), [&](const PaxosNodeDescriptor& n){
        return n.node_id == node.node_id;
    }), cluster_nodes.end());

    std::cout<<"[INFO] Node removed: "<< node.node_id<<"\n";
}

void PaxosNode::receive_heart_beat(HeartBeatMessage msg){
    msg.log_heartbeat();
    // std::cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n";
    // std::cout<<msg.node_id<<"\n";
    if(!update_last_seen(msg)){
        // probably a new node, so add it to the list
        PaxosNodeSharableDescriptor new_node{
            msg.node_id,
            msg.node_ip,
            msg.node_port
        };

        add_new_node(new_node, msg.cluster_type);
    }
}

bool PaxosNode::send_message(const std::string& node_ip, int node_port, const Message& msg){
    return rpc_client.send(node_ip, node_port, msg);
}

bool PaxosNode::send_heart_beat(std::string node_ip, int node_port, HeartBeatMessage msg){
    // std::cout<<"Sending Heartbeat: "<<msg.node_id<<"\n";
    return send_message(node_ip, node_port, msg);
}

void PaxosNode::broadcast_heart_beat(const HeartBeatMessage base_msg, int cluster_type){
    std::vector<std::future<void>> tasks;

    if (cluster_type == 1){
        // cluster
        for(const auto& node: cluster_nodes){
            if (node.node_id != this->node_id){
                tasks.push_back(std::async(std::launch::async, [this, node, base_msg](){
                    // HeartBeatMessage msg = base_msg;
                    // msg.node_id = this->node_id;
                    // msg.node_ip = this->node_ip;
                    // msg.node_port = this->node_port;
                    // send_heart_beat(node.node_ip, node.node_port, msg);
                    send_heart_beat(node.node_ip, node.node_port, base_msg);
                }));
            }
        }
    }else{
            // local cluster
            for(const auto& node: local_cluster_nodes){
                if (node.node_id != this->node_id){
                    tasks.push_back(std::async(std::launch::async, [this, node, base_msg](){
                        // HeartBeatMessage msg = base_msg;
                        // msg.node_id = this->node_id;
                        // msg.node_ip = this->node_ip;
                        // msg.node_port = this->node_port;
                        // send_heart_beat(node.node_ip, node.node_port, msg);
                        send_heart_beat(node.node_ip, node.node_port, base_msg);
                    }));
                }
            
            }
    }

    for(auto& task: tasks){
        task.get();
    }
}

void PaxosNode::join_cluster(const std::vector<PaxosNodeSharableDescriptor>& initial_nodes) {
    for (const auto& node : initial_nodes) {
        if (node.node_id != this->node_id ) {
            add_new_node(node, 1);
        }
    }
}

void PaxosNode::register_cluster_nodes(const std::vector<PaxosNodeSharableDescriptor>& initial_nodes) {
    for (const auto& node : initial_nodes) {
        add_new_node(node, 2);
    }
}
