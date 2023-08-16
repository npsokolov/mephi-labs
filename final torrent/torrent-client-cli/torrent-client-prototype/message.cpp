#include "message.h"
#include "byte_tools.h"
#include <cstdint>

Message Message::Parse(const std::string& messageString){
    Message message;
    if (messageString.size()==0){
        message.id =MessageId::KeepAlive;
        message.messageLength=0;
    }else{
        int ID;
        message.id = MessageId(int(messageString[0]));
        message.messageLength=messageString.size();
        message.payload=messageString.substr(1);
    }
    return message;
}

Message Message::Init(MessageId id, const std::string& payload){
    Message message;
    message.id=id;
    message.payload=payload;
    if (id==MessageId::KeepAlive){
        message.messageLength = 0;
        return message;
    }
    message.messageLength=payload.size()+1;
    return message;
}

std::string Message::ToString() const{
    if (id!=MessageId::KeepAlive){
        std::string return_value(IntToBytes(messageLength));
        return_value.push_back((unsigned char)(int(id)));
        return_value+=payload;
        return return_value;
    }else{

        std::string return_value;
        for (size_t i = 0; i < 4; i++)
        {
            return_value.push_back((unsigned char)0);
        }
        return return_value;
    }
}