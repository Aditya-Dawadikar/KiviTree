#pragma once
#include <string>

enum MessageType{
    PREPARE,
    PROMISE,
    ACCEPT,
    ACCEPTED,
    HEART_BEAT
};

class Message{
    private:
    public:
        virtual std::string serialize() const = 0;
        virtual MessageType get_message_type() const = 0;
        virtual ~Message(){}

};