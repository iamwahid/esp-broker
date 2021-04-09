#ifndef _LISTENER_BASE_H_
#define _LISTENER_BASE_H_

#include "../system/EventMQ.h"
#include "../system/helpers.h"
#include "../system/defines.h"
#include <Ticker.h>
#include <pins_arduino.h>
#include "../config.h"
// #include <Preferences.h>

namespace Modules {

MessageObject rx_Msg; // original message
MessageObject tmp_Msg; // temp message to change on condition

class Listener {
  public:
  ListenerType MODULE_TYPE;
  Listener() {};
  Listener(ListenerType MODULE_TYPE): MODULE_TYPE(MODULE_TYPE) {};

  virtual ~Listener(){};

  virtual void initListener() {

  }

  void begin() {
    // WiFi.persistent(false);
    // WiFi.mode(WIFI_STA);
    // WiFi.softAP("ESPNOW", nullptr, 3);
    // WiFi.softAPdisconnect(false);
    // WiFi.disconnect();
    bool ok = EspNowMQ.begin(NET_ROLE::LISTENER, "mbayar0");
    if (!ok) {
      Serial.println("failed");
      ESP.restart();
    }

    EspNowMQ.setMaster(masterMac);

    pinMode(DEFAULT_BUTTON, INPUT_PULLUP);
    SET_PIN(MODULE_TYPE);

    initListener();
  }

  virtual void loop() {

  };

}; // end Listener

}


#endif