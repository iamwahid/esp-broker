// #ifndef _MQ_EVENTS_TYPE_
// #define _MQ_EVENTS_TYPE_

// #include <EspNowMQ.h>
// #include <map>
// #include <memory>
// #include "globalConf.h"

// #define EACH_MATCH(f)                                                                              \
//   do {                                                                                             \
//       for (int i = 0; i < EspNowMQ.listenerCount; i++) {                                           \
//           if (EspNowMQ.peerListeners[i].listenOn != rx_Msg.m_event_name) continue;                 \
//           if (EspNowMQ.hasPeer(EspNowMQ.peerListeners[i].mac) &&                                   \
//               (EspNowMQ.peerListeners[i].type == ListenerType::OUTPUT_ANALOG ||                    \
//               EspNowMQ.peerListeners[i].type == ListenerType::OUTPUT_BINARY)) {                    \
//               (f)(EspNowMQ.peerListeners[i]);                                                      \
//           }                                                                                        \
//       }                                                                                            \
//     } while (false)

// void macToHex(uint8_t * dst, char * str) {
//   // char str[] = "B4:21:8A:F0:3A:AD";
//   // uint8_t MAC[6];
//   char* ptr;

//   dst[0] = strtol(str, &ptr, HEX );
//   for( uint8_t i = 1; i < 6; i++ )
//   {
//     dst[i] = strtol(ptr+1, &ptr, HEX );
//   }
//   // DEBUG
//   // Serial.print(dst[0], HEX);
//   // for( uint8_t i = 1; i < 6; i++)
//   // {
//   //   Serial.print(':');
//   //   Serial.print( dst[i], HEX);
//   // }

//   // Serial.println();
//   // Serial.println(str);
// }

// class LogicObject {
//   public:
//   LogicObject() {};
//   LogicObject(int value): value(value) {}
//   virtual ~LogicObject() {};

//   int &getValue() { return value;}
//   void updateObject() {

//   }

//   private:
//   int value = 0;
//   EspNowMQPeerInfo peer;

// };

// class EventObject {
//   public:
//   MessageObject rx_Msg; // original message
//   // std::vector<MessageObject> Msgs;
//   // std::map<uint8_t *, std::shared_ptr<LogicObject>> logics; 
//   std::map<uint8_t *, std::shared_ptr<DataObject>> dataMap;
//   LogicObject logic;
//   String logicChain;
//   MessageObject result; // temp message to change on condition

//   EventObject() {};
  
//   virtual ~EventObject() {};

//   void setLogic(String logic) {
//     logicChain = logic;
//   }

//   virtual void logical() {
//     // if (Msgs.size()>0) {
//     //   memcpy(&result, &Msgs[0], sizeof(result));
//     //   for (MessageObject m : Msgs) {
//     //     result.data.value += m.data.value;
//     //   }
//     // }
//   }

//   virtual void preProcess() {

//   }

//   virtual bool postProcess(EspNowMQPeerInfo peer, MessageObject msg) {
//     // add depends on logicalobject count
//     // logics.emplace(peer.mac, std::make_shared<LogicObject>(peer.mac, msg.data.value));
//     dataMap.emplace(peer.mac, std::make_shared<DataObject>(msg.data.name, msg.data.value));

//     // Msgs.push_back(msg);
//     // tmp_Msg.size();
//     logical();
//     return true;
//   }

//   void process(const uint8_t* data, uint8_t size) {
//     memcpy(&rx_Msg, data, sizeof(rx_Msg));
//     preProcess();
//     EACH_MATCH([&](EspNowMQPeerInfo peer) {
//         // keep original rxMsg
//         if (postProcess(peer, rx_Msg)) {
//           // send copied and modified messageObject
//           if (EspNowMQ.send(peer.mac, (uint8_t *)&result, sizeof(result))) {
//             Serial.println("Send Success");
//             Serial.println(result.data.name);
//           } else {
//             Serial.println("Send Failed");
//           }
//         }
//     });
//   }
// };

// class SwitchEvent: public EventObject {
//   public:
//   SwitchEvent(): EventObject() {}

//   virtual bool postProcess(EspNowMQPeerInfo peer, MessageObject msg) {
//     memcpy(&result, &msg, sizeof(result));
//     Serial.println(result.data.name);
//     if (EspNowMQ.equals(result.src, peer.mac, 6)) return false;
//     return true;
//   }

//   virtual void preProcess() {
//     Serial.print("EVENT: ");
//     Serial.print(rx_Msg.m_event_name);
//     Serial.print(" NAME: ");
//     Serial.print(rx_Msg.data.name);
//     Serial.print(" VALUE: ");
//     Serial.println(rx_Msg.data.value);
//   }
// };

// class BlinkEvent: public EventObject {
//   public:
//   BlinkEvent(): EventObject() {}
  
//   virtual bool postProcess(EspNowMQPeerInfo peer, MessageObject msg) {
//     // memcpy(&result, &msg, sizeof(result));

//     // if (result.data.value > 100) {
//     //   if (peer.type == ListenerType::OUTPUT_ANALOG) {
//     //     result.data.value = 1000;
//     //   } else if (peer.type == ListenerType::OUTPUT_BINARY) {
//     //     result.data.value = 1;
//     //   }
//     // } else if (result.data.value <= 100) {
//     //   if (peer.type == ListenerType::OUTPUT_ANALOG) {
//     //     result.data.value = 500;
//     //   } else if (peer.type == ListenerType::OUTPUT_BINARY) {
//     //     result.data.value = 0;
//     //   }
//     // }
//     return true;
//   }

//   void preBlink(EspNowMQPeerInfo peer, MessageObject msg, MessageObject * storeTo) {
//     memcpy(&storeTo, &msg, sizeof(storeTo));
    
//     if (storeTo->data.value > 100) {
//       if (peer.type == ListenerType::OUTPUT_ANALOG) {
//         storeTo->data.value = 1000;
//       } else if (peer.type == ListenerType::OUTPUT_BINARY) {
//         storeTo->data.value = 1;
//       }
//     } else if (storeTo->data.value <= 100) {
//       if (peer.type == ListenerType::OUTPUT_ANALOG) {
//         storeTo->data.value = 500;
//       } else if (peer.type == ListenerType::OUTPUT_BINARY) {
//         storeTo->data.value = 0;
//       }
//     }
//   }
// };

// #endif