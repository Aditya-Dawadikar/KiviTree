#pragma once
#include "kivitree_utils/message.hpp"
#include "kivitree_utils/json.hpp"
#include "kivileaf/local_sync_payload.hpp"
#include <unordered_map>
using json = nlohmann::json;

class LocalSyncSelfPromotionMessage:public Message{
    public:
        long long int leader_node_id;
        long long int promotion_timestamp;

        LocalSyncSelfPromotionMessage(long long int leader_node_id,
                             long long promotion_timestamp)
        : leader_node_id(leader_node_id),
          promotion_timestamp(promotion_timestamp){}

        MessageType get_message_type() const{
            return MessageType::LOCAL_SYNC_SELF_PROMOTION;
        }

        std::string serialize() const{
            json j;
            j["type"] = static_cast<int>(get_message_type());
            j["leader_node_id"] = leader_node_id;
            j["promotion_timestamp"] = promotion_timestamp;
            return j.dump();
        }

        static std::unique_ptr<Message> deserialize(const std::string& json_str){
            json j = json::parse(json_str);
            return std::make_unique<LocalSyncSelfPromotionMessage>(
                j.at("leader_node_id").get<long long int>(),
                j.at("promotion_timestamp").get<long long int>()
            );
        }
};