#pragma once
#include "kivitree_utils/message.hpp"
#include "kivitree_utils/json.hpp"
#include <unordered_map>
using json = nlohmann::json;

class LocalSyncRequestMessage:public Message{

    public:
        
        long long int follower_node_id;
        long long int last_sync_timestamp;

        LocalSyncRequestMessage(long long int follower_node_id, long long int last_sync_timestamp)
                                :follower_node_id(follower_node_id), last_sync_timestamp(last_sync_timestamp){};

        MessageType get_message_type() const{
            return MessageType::LOCAL_SYNC_REQUEST;
        }

        std::string serialize() const{
            json j;
            j["type"] = static_cast<int>(get_message_type());
            j["follower_node_id"] = follower_node_id;
            j["last_sync_timestamp"] = last_sync_timestamp;
            return j.dump();
        }

        static std::unique_ptr<Message> deserialize(const std::string& json_str){
            json j = json::parse(json_str);
            return std::make_unique<LocalSyncRequestMessage>(
                j.at("follower_node_id").get<long long int>(),
                j.at("last_sync_timestamp").get<long long int>()
            );
        }
};