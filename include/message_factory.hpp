#pragma once
#include <memory>
#include <string>
#include "message.hpp"

class MessageFactory {
public:
    static std::unique_ptr<Message> from_json(const std::string& json_str);
};
