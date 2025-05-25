#include "paxos_message.hpp"
#include "json.hpp"
using json = nlohmann::json;

std::string PaxosMessage::serialize() const{
    json j;
    j["type"] = static_cast<int>(type);
    j["key"] = key;
    j["value"] = value;
    j["proposal_number"] = proposal_number;
    j["proposer_id"] = proposer_id;
    j["timestamp"] = timestamp;
    return j.dump();
}
PaxosMessage PaxosMessage::deserialize(const std::string& json_str){
    json j = json::parse(json_str);
    PaxosMessage msg;
    msg.type = static_cast<MessageType>(j["type"].get<int>());
    // msg.key = j.value("key", "");
    // msg.value = j.value("value", "");
    // msg.proposer_id = j.value("proposer_id", "");
    // msg.proposal_number = j.value("proposal_number", 0);
    // msg.proposal_number = j.value("timestamp", 0);

    msg.key = j.at("key").get<std::string>();
    msg.value = j.at("value").get<std::string>();
    msg.proposal_number = j.at("proposal_number").get<int>();
    msg.proposer_id = j.at("proposer_id").get<std::string>();
    msg.timestamp = j.at("timestamp").get<long long>();

    return msg;
}