#include "../core/event_mq.h"

namespace Events {

struct BlinkEvent: EventBrokerObject {
  public:
  long interval = 1000;
  long val = 0;
  BlinkEvent(): EventBrokerObject() {}

  void preProcess() override {
    pause();
    Serial.print("EVENT: ");
    Serial.print(rx_Msg.m_event_name);
    Serial.print(" NAME: ");
    Serial.print(rx_Msg.data.name);
    Serial.print(" VALUE: ");
    Serial.println(rx_Msg.data.toString());
    repeat(rx_Msg.data.toInt());
  }

  bool postProcess(WifiNowPeer &peer, MessageObject &msg) override {
    val = !val;
    memcpy(&result, &msg, sizeof(result));
    result.data.setFloat(val);
    Serial.println(val);
    return true;
  }

};

}
