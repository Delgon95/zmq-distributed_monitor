#include "message.h"

Message::Message(MessageType _type,
                 const std::string& _monitor_id,
                 const std::string& _sending_server,
                 const std::string& _data)
    : type(_type), monitor_id(_monitor_id),
      sending_server(_sending_server), data(_data) {}

Dispatch::Dispatch(const Message& _message,
                   const std::string& _destination)
    : message(_message), destination(_destination) {}
