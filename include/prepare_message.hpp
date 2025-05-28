#pragma once
#include "message.hpp"
#include "json.hpp"
using json = nlohmann::json;

class PrepareMessage:public Message{
    private:
        std::string proposer_id;
        long long int proposal_number;
        std::vector<std::string> keys;
        long long int timestamp;
    public:
        PrepareMessage() = default;

        PrepareMessage(std::string id, long long proposal, std::vector<std::string> k, long long ts)
        : proposer_id(id), proposal_number(proposal), keys(k), timestamp(ts) {}

        MessageType get_message_type() const{
            return MessageType::PREPARE;
        }

        std::string serialize() const{
            json j;
            j["type"] = static_cast<int>(get_message_type());
            j["keys"] = keys;
            j["proposal_number"] = proposal_number;
            j["proposer_id"] = proposer_id;
            j["timestamp"] = timestamp;
            return j.dump();
        }

        static std::unique_ptr<Message> deserialize(const std::string& json_str){
            json j = json::parse(json_str);
            return std::make_unique<PrepareMessage>(
                j.at("proposer_id").get<std::string>(),
                j.at("proposal_number").get<long long>(),
                j.at("keys").get<std::vector<std::string>>(),
                j.at("timestamp").get<long long>()
            );
        }
};