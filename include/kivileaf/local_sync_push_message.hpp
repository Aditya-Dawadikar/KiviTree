#pragma once
#include "kivitree_utils/message.hpp"
#include "kivitree_utils/json.hpp"
#include "kivileaf/local_sync_payload.hpp"
#include <unordered_map>
using json = nlohmann::json;

class LocalSyncPushMessage:public Message{
    public:
        long long int leader_node_id;
        long long int latest_sync_timestamp;
        std::string key;
        std::string value;

        LocalSyncPushMessage(long long int leader_node_id,
                             long long latest_sync_timestamp,
                             std::string key,
                             std::string value)
        : leader_node_id(leader_node_id),
          latest_sync_timestamp(latest_sync_timestamp),
          key(key),
          value(value) {}

        MessageType get_message_type() const{
            return MessageType::LOCAL_SYNC_PUSH;
        }

        std::string serialize() const{
            json j;
            j["type"] = static_cast<int>(get_message_type());
            j["leader_node_id"] = leader_node_id;
            j["latest_sync_timestamp"] = latest_sync_timestamp;
            j["key"] = key;
            j["value"]= value;
            return j.dump();
        }

        static std::unique_ptr<Message> deserialize(const std::string& json_str){
            json j = json::parse(json_str);
            return std::make_unique<LocalSyncPushMessage>(
                j.at("leader_node_id").get<long long int>(),
                j.at("latest_sync_timestamp").get<long long int>(),
                j.at("key").get<std::string>(),
                j.at("value").get<std::string>()
            );
        }
};