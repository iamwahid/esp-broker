#include "../system/EventMQ.h"

namespace Events {

struct SwitchEvent: EventBrokerObject {
  public:
  SwitchEvent(): EventBrokerObject() {}

  bool postProcess(EspNowMQPeerInfo &peer, MessageObject &msg) override {
    memcpy(&result, &msg, sizeof(msg));
    Serial.println(result.data.name);
    if (EspNowMQ.equals(result.src, peer.mac, 6)) return false;
    return true;
  }

  void preProcess() override {
    Serial.print("EVENT: ");
    Serial.print(rx_Msg.m_event_name);
    Serial.print(" NAME: ");
    Serial.print(rx_Msg.data.name);
    Serial.print(" VALUE: ");
    Serial.println(rx_Msg.data.toString());
  }
};

}