#pragma once

#include "../core/EventMQ.h"
#include "../core/helpers.h"
#include "../core/defines.h"
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
  String SubTo;
  String PubTo;
  Listener() {};
  Listener(ListenerType MODULE_TYPE): MODULE_TYPE(MODULE_TYPE) {};

  virtual ~Listener(){};

  virtual void initListener() {

  }

  void begin() {
    bool ok = EspNowMQ.begin(NET_ROLE::LISTENER);
    if (!ok) {
      Serial.println("failed");
      ESP.restart();
    }

    EspNowMQ.setMaster(masterMac);

    pinMode(DEFAULT_BUTTON, INPUT_PULLUP);
    SET_PIN(MODULE_TYPE);

    this->initListener();
  }

  virtual void loop() {

  };

}; // end Listener

}


