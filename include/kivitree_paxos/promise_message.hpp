#pragma once
#include "kivitree_utils/message.hpp"
#include "kivitree_utils/json.hpp"
using json = nlohmann::json;

class PromiseMessage:public Message{
    private:
        long long int proposer_id;
        long long int proposal_number;
        std::vector<std::string> keys;
        long long int timestamp;
    public:
        PromiseMessage() = default;

        PromiseMessage(long long int id, long long proposal, std::vector<std::string> k, long long ts)
        : proposer_id(id), proposal_number(proposal), keys(k), timestamp(ts) {}

        MessageType get_message_type() const{
            return MessageType::PROMISE;
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
            return std::make_unique<PromiseMessage>(
                j.at("proposer_id").get<long long int>(),
                j.at("proposal_number").get<long long>(),
                j.at("keys").get<std::vector<std::string>>(),
                j.at("timestamp").get<long long>()
            );
        }
};