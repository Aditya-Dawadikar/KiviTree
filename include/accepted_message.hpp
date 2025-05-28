#pragma once
#include "message.hpp"
#include "json.hpp"
#include <unordered_map>
using json = nlohmann::json;

class AcceptedMessage:public Message{
    private:
        std::string proposer_id;
        long long int proposal_number;
        std::unordered_map<std::string, std::string> kv_batch;
        long long int timestamp;
    public:
        AcceptedMessage() = default;

        AcceptedMessage(std::string id, long long proposal, std::unordered_map<std::string, std::string> kv_batch, long long ts)
        : proposer_id(id), proposal_number(proposal), kv_batch(std::move(kv_batch)), timestamp(ts) {}

        MessageType get_message_type() const{
            return MessageType::ACCEPTED;
        }

        std::string serialize() const{
            json j;
            j["type"] = static_cast<int>(get_message_type());
            j["kv_batch"] = kv_batch;
            j["proposal_number"] = proposal_number;
            j["proposer_id"] = proposer_id;
            j["timestamp"] = timestamp;
            return j.dump();
        }

        static std::unique_ptr<Message> deserialize(const std::string& json_str){
            json j = json::parse(json_str);
            return std::make_unique<AcceptedMessage>(
                j.at("proposer_id").get<std::string>(),
                j.at("proposal_number").get<long long>(),
                j.at("kv_batch").get<std::unordered_map<std::string, std::string>>(),
                j.at("timestamp").get<long long>()
            );
        }
};