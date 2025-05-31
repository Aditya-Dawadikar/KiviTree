#pragma once
#include "kivitree_utils/message.hpp"
#include "kivitree_utils/json.hpp"
#include "kivitree_utils/timestamp.hpp"
#include <iostream>
using json = nlohmann::json;

class HeartBeatMessage:public Message{
    public:
        long long int node_id;
        std::string node_ip;
        int node_port;
        long long last_seen;
        int cluster_type;

        HeartBeatMessage() = default;

        HeartBeatMessage(long long int node_id, std::string node_ip, int node_port, long long last_seen, int cluster_type)
        : node_id(node_id), node_ip(node_ip), node_port(node_port), last_seen(last_seen), cluster_type(cluster_type){}

        MessageType get_message_type() const{
            return MessageType::HEART_BEAT;
        }

        std::string serialize() const{
            json j;
            j["type"] = static_cast<int>(get_message_type());
            j["node_id"] = node_id;
            j["node_ip"] = node_ip;
            j["node_port"] = node_port;
            j["last_seen"] = last_seen;
            j["cluster_type"] = cluster_type;
            return j.dump();
        }

        static std::unique_ptr<Message> deserialize(const std::string& json_str){
            json j = json::parse(json_str);
            // std::cout<<"=======================================\n";
            // std::cout<<j.dump()<<"\n";
            return std::make_unique<HeartBeatMessage>(
                j.at("node_id").get<long long int>(),
                j.at("node_ip").get<std::string>(),
                j.at("node_port").get<int>(),
                j.at("last_seen").get<long long>(),
                j.at("cluster_type").get<int>()
            );
        }

        void log_heartbeat(){
            std::cout<<"[Heartbeat]("<<Timestamp::now_ms()<<")"<<" From Host: "<<this->node_ip<<":"<<this->node_port<<"("<<this->node_id<<")"<<"\n";
        }
};