#pragma once
#include <memory>
#include <string>
#include "kivitree_utils/message.hpp"

class MessageFactory {
public:
    static std::unique_ptr<Message> from_json(const std::string& json_str);
};
