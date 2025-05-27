#pragma once
#include "paxos_message.hpp"

class Message{
    private:
    public:
        virtual std::string serialize() const = 0;
        virtual MessageType get_message_type() const = 0;
        virtual ~Message(){}

};