#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <string>       // string
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/JsonThor.h"

enum MessageType {
  RequestToken,
  RequestBarrier,
  RequestWait,
  RespondToken,
  RespondBarrier,
  RespondNotifyAll,
  EndProgram
};

struct Message {
  Message() = default;
  Message(MessageType _type,
          const std::string& _monitor_id,
          const std::string& _sending_server,
          const std::string& _data);
  MessageType type;
  std::string monitor_id;
  std::string sending_server;
  std::string data;

  // Define the trait as a frient to get accesses to private members.
  friend class ThorsAnvil::Serialize::Traits<Message>;
};

// Declare the traits.
// Speifying what members need to be serialized.
ThorsAnvil_MakeEnum(MessageType, RequestToken, RequestBarrier, RequestWait,
                    RespondToken, RespondBarrier, RespondNotifyAll, EndProgram);
ThorsAnvil_MakeTrait(Message, type, monitor_id, sending_server, data);

struct Dispatch {
  Dispatch() = default;
  Dispatch(const Message& _message, const std::string& _destination);

  Message message;
  std::string destination;
};

#endif // MESSAGE_H_
