#pragma once
#include "kivitree_utils/message.hpp"
#include "kivitree_utils/json.hpp"
#include "kivileaf/local_sync_payload.hpp"
#include <unordered_map>
using json = nlohmann::json;

class LocalSyncResponseMessage:public Message{
    public:
        long long int leader_node_id;
        long long int follower_node_id;
        long long int last_sync_timestamp;
        long long int latest_sync_timestamp;
        LocalSyncPayload payload;

        LocalSyncResponseMessage(long long follower_id,
                             long long last_sync_timestamp,
                             long long latest_sync_timestamp,
                             const LocalSyncPayload& p)
        : leader_node_id(leader_node_id),
          follower_node_id(follower_id),
          last_sync_timestamp(last_sync_timestamp),
          latest_sync_timestamp(latest_sync_timestamp),
          payload(p) {}

        MessageType get_message_type() const{
            return MessageType::LOCAL_SYNC_RESPONSE;
        }

        std::string serialize() const{
            json j;
            j["type"] = static_cast<int>(get_message_type());
            j["follower_node_id"] = follower_node_id;
            j["last_sync_timestamp"] = last_sync_timestamp;
            j["latest_sync_timestamp"] = latest_sync_timestamp;
            j["payload"] = payload.dump();
            return j.dump();
        }

        static std::unique_ptr<Message> deserialize(const std::string& json_str){
            json j = json::parse(json_str);
            LocalSyncPayload payload;
            payload.load(j.at("payload").get<std::string>());
            return std::make_unique<LocalSyncResponseMessage>(
                j.at("follower_node_id").get<long long int>(),
                j.at("last_sync_timestamp").get<long long int>(),
                j.at("latest_sync_timestamp").get<long long int>(),
                payload
            );
        }
};