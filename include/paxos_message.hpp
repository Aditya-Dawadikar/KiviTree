#pragma once
#include <string>

enum class MessageType{
    PREPARE,
    PROMISE,
    ACCEPT,
    ACCEPTED
};

struct PaxosMessage{
    MessageType type;
    std::string key;
    std::string value;
    int proposal_number;
    std::string proposer_id;
    long long timestamp;

    std::string serialize() const;
    static PaxosMessage deserialize(const std::string& json_str);
};