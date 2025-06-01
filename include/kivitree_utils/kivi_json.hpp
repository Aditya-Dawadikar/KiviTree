#pragma once
#include "kivitree_paxos/paxos_node.hpp"
#include <kivitree_utils/json.hpp>

inline nlohmann::json to_json(const PaxosNode::PaxosNodeDescriptor& node) {
    return {
        {"node_id", node.node_id},
        {"node_ip", node.node_ip},
        {"node_port", node.node_port},
        {"last_seen", node.last_seen},
        {"last_sync", node.last_sync},
        {"is_alive", node.is_alive}
    };
}
