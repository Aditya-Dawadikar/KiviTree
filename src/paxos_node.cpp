#include "paxos_node.hpp"
#include "rpc_server.hpp"
#include "rpc_client.hpp"
#include "timestamp.hpp"
#include <thread>
#include <iostream>
#include "message.hpp"
#include "future"
#include "prepare_message.hpp"
#include "promise_message.hpp"
#include "accept_message.hpp"
#include "accepted_message.hpp"
#include "heartbeat_message.hpp"

PaxosNode::PaxosNode(std::string node_id, std::string node_ip, int node_port, bool is_proposer)
                    :node_id(node_id),node_ip(node_ip), node_port(node_port), is_proposer(is_proposer){}

void PaxosNode::start(int port,
            std::function<void(const Message&)> handler){
    
    this->node_port = port;

    // background thread to send heartbeat every 5s
    std::thread([this](){
        try{
            std::cout << "[THREAD] Heartbeat thread running\n";
            while(true){
                HeartBeatMessage msg;

                msg.node_id = this->node_id;
                msg.node_ip = this->node_ip;
                msg.node_port = this->node_port;
                msg.last_seen = Timestamp::now_ms();

                broadcast_heart_beat(msg);

                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        } catch (const std::exception& e) {
            std::cerr << "[EXCEPTION] Heartbeat thread crashed: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "[EXCEPTION] Heartbeat thread crashed with unknown error.\n";
        }
    }).detach();

    std::cout << "[INFO] Heartbeat thread launched successfully\n";

    // Start failure detector
    std::thread([this](){
        while(true){
            long long now = Timestamp::now_ms();
            for(auto& node: cluster_nodes){
                if(node.node_id == this->node_id) {
                    node.is_alive = true;
                    continue;
                };

                if(now - node.last_seen > 15000 && node.is_alive){
                    // mark node as dead
                    node.is_alive = false;
                    std::cout << "[DEAD] Node "<< node.node_id<<" marked as DEAD\n";
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }).detach();

    RPCServer::start(port, [this, handler](const Message& msg) {
        switch (msg.get_message_type()) {
            case MessageType::HEART_BEAT: {
                if (auto hb_msg = dynamic_cast<const HeartBeatMessage*>(&msg)) {
                    this->receive_heart_beat(*hb_msg);
                } else {
                    std::cerr << "[ERROR] Failed to cast to HeartBeatMessage\n";
                }
                break;
            }

            case MessageType::PREPARE: {
                if (auto prep = dynamic_cast<const PrepareMessage*>(&msg)) {
                    handler(*prep);
                } else {
                    std::cerr << "[ERROR] Failed to cast to PrepareMessage\n";
                }
                break;
            }

            case MessageType::PROMISE: {
                if (auto prom = dynamic_cast<const PromiseMessage*>(&msg)) {
                    handler(*prom);
                } else {
                    std::cerr << "[ERROR] Failed to cast to PromiseMessage\n";
                }
                break;
            }

            case MessageType::ACCEPT: {
                if (auto acc = dynamic_cast<const AcceptMessage*>(&msg)) {
                    handler(*acc);
                } else {
                    std::cerr << "[ERROR] Failed to cast to AcceptMessage\n";
                }
                break;
            }

            case MessageType::ACCEPTED: {
                if (auto accd = dynamic_cast<const AcceptedMessage*>(&msg)) {
                    handler(*accd);
                } else {
                    std::cerr << "[ERROR] Failed to cast to AcceptedMessage\n";
                }
                break;
            }

            default:
                std::cerr << "[ERROR] Unknown message type received\n";
        }
    });

    std::cout << "[NODE] Started node "<<node_id<<" on port "<<port<<"\n";

}

bool PaxosNode::is_node_alive(const std::string& node_id){
    for(const auto& node:cluster_nodes){
        if (node.node_id == node_id){
            return node.is_alive;
        }
    }
    return false;   //unknown node
}

bool PaxosNode::update_last_seen(const HeartBeatMessage& msg){
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
    return false;
}

void PaxosNode::add_new_node(PaxosNodeSharableDescriptor new_node){

    PaxosNodeDescriptor desc;
    desc.node_id = new_node.node_id;
    desc.node_ip = new_node.node_ip;
    desc.node_port = new_node.node_port;
    desc.last_seen = Timestamp::now_ms();
    desc.is_alive = true;

    cluster_nodes.push_back(desc);
    std::cout<<"[INFO] New node added to cluster: "<<new_node.node_id <<"\n";
}

void PaxosNode::remove_node(PaxosNodeSharableDescriptor node){
    cluster_nodes.erase(std::remove_if(cluster_nodes.begin(), cluster_nodes.end(), [&](const PaxosNodeDescriptor& n){
        return n.node_id == node.node_id;
    }), cluster_nodes.end());

    std::cout<<"[INFO] Node removed: "<< node.node_id<<"\n";
}

void PaxosNode::receive_heart_beat(HeartBeatMessage msg){
    msg.log_heartbeat();
    if(!update_last_seen(msg)){
        // probably a new node, so add it to the list
        PaxosNodeSharableDescriptor new_node{
            msg.node_id,
            msg.node_ip,
            msg.node_port
        };
        add_new_node(new_node);
    }
}

bool PaxosNode::send_message(const std::string& node_ip,
                                int node_port,
                                const Message& msg){
    return rpc_client.send(node_ip, node_port, msg);
}

bool PaxosNode::send_heart_beat(std::string node_ip, int node_port, HeartBeatMessage msg){
    // msg.log_heartbeat();
    // std::cout<<"node_ip:"<<node_ip<<", node_port:"<<node_port<<"\n";
    // std::cout<<"Sending Heartbeat to: "<<node_ip<<":"<<node_port<<"\n";
    return send_message(node_ip, node_port, msg);
}

void PaxosNode::broadcast_heart_beat(const HeartBeatMessage base_msg){
    std::vector<std::future<void>> tasks;
    
    for(const auto& node: cluster_nodes){

        if (node.node_id != this->node_id && node.is_alive){
            tasks.push_back(std::async(std::launch::async, [this, node, base_msg](){
                HeartBeatMessage msg = base_msg;
                msg.node_ip = this->node_ip;
                msg.node_port = this->node_port;
                send_heart_beat(node.node_ip, node.node_port, msg);
            }));
        }
        
    }

    for(auto& task: tasks){
        task.get();
    }
}

void PaxosNode::join_cluster(const std::vector<PaxosNodeSharableDescriptor>& initial_nodes) {
    for (const auto& node : initial_nodes) {
        if (node.node_id != this->node_id ) {
            add_new_node(node);
        }
    }
}
