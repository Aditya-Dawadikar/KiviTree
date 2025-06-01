#include "kivitree_paxos/message_factory.hpp"
#include "kivitree_paxos/heartbeat_message.hpp"
#include "kivitree_paxos/prepare_message.hpp"
#include "kivitree_paxos/promise_message.hpp"
#include "kivitree_paxos/accept_message.hpp"
#include "kivitree_paxos/accepted_message.hpp"
#include "kivitree_utils/json.hpp"
#include "kivileaf/local_sync_push_message.hpp"
#include "kivileaf/local_sync_request_message.hpp"
#include "kivileaf/local_sync_response_message.hpp"

using json = nlohmann::json;

std::unique_ptr<Message> MessageFactory::from_json(const std::string& json_str) {
    json j = json::parse(json_str);
    MessageType type = static_cast<MessageType>(j.at("type").get<int>());

    switch (type) {
        case MessageType::HEART_BEAT:
            return HeartBeatMessage::deserialize(json_str);
        case MessageType::PREPARE:
            return PrepareMessage::deserialize(json_str);
        case MessageType::PROMISE:
            return PromiseMessage::deserialize(json_str);
        case MessageType::ACCEPT:
            return AcceptMessage::deserialize(json_str);
        case MessageType::ACCEPTED:
            return AcceptedMessage::deserialize(json_str);
        case MessageType::LOCAL_SYNC_REQUEST:
            return LocalSyncRequestMessage::deserialize(json_str);
        case MessageType::LOCAL_SYNC_RESPONSE:
            return LocalSyncResponseMessage::deserialize(json_str);
        case MessageType::LOCAL_SYNC_PUSH:
            return LocalSyncPushMessage::deserialize(json_str);
        default:
            throw std::runtime_error("Unknown message type");
    }
}
