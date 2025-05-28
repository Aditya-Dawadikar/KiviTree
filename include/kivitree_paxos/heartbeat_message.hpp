#pragma once
#include "kivitree_utils/message.hpp"
#include "kivitree_utils/json.hpp"
#include "kivitree_utils/timestamp.hpp"
#include <iostream>
using json = nlohmann::json;

class HeartBeatMessage:public Message{
    public:
        std::string node_id;
        std::string node_ip;
        int node_port;
        long long last_seen;

        HeartBeatMessage() = default;

        HeartBeatMessage(std::string node_id, std::string node_ip, int node_port, long long last_seen)
        : node_id(node_id), node_ip(node_ip), node_port(node_port), last_seen(last_seen) {}

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
            return j.dump();
        }

        static std::unique_ptr<Message> deserialize(const std::string& json_str){
            json j = json::parse(json_str);
            return std::make_unique<HeartBeatMessage>(
                j.at("node_id").get<std::string>(),
                j.at("node_ip").get<std::string>(),
                j.at("node_port").get<int>(),
                j.at("last_seen").get<long long>()
            );
        }

        void log_heartbeat(){
            std::cout<<"[Heartbeat]("<<Timestamp::now_ms()<<")"<<" From Host: "<<this->node_ip<<":"<<this->node_port<<"("<<this->node_id<<")"<<"\n";
        }
};