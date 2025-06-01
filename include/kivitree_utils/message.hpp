#pragma once
#include <string>

enum MessageType{
    PREPARE,
    PROMISE,
    ACCEPT,
    ACCEPTED,
    HEART_BEAT,
    LOCAL_SYNC_REQUEST,
    LOCAL_SYNC_RESPONSE,
    LOCAL_SYNC_PUSH
};

class Message{
    private:
    public:
        virtual std::string serialize() const = 0;
        virtual MessageType get_message_type() const = 0;
        virtual ~Message(){}

};