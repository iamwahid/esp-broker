#include "../../core/listener.h"

namespace Modules {

// save last state
class SwitchPub: public Listener {
  public:
  int Val = 100;
  SwitchPub(): Listener(ListenerType::INPUT_BINARY) {}
  void loop() override {
    // if (digitalRead(DEFAULT_BUTTON) == LOW) { // button is pressed
    //   bool sent = EspNowMQ.send("SWITCH", masterMac, "SWITCH1", 1);
    //   if (sent) {
    //     Serial.println("Send Success");
    //   } else {
    //     Serial.println("Send Failed");
    //   }

    //   while (digitalRead(DEFAULT_BUTTON) == LOW) // wait for button release
    //     ;
    // }
    // delay(200);
    if (Val < 2000) {
      Val += 10;
    } else {
      Val = 100;
    }
    bool sent = EspNowMQ.send("SWITCH", masterMac, "SWITCH1", (int *)Val);
    if (sent) {
      Serial.println("Send Success");
    } else {
      Serial.println("Send Failed");
    }
  }
}; // end SwitchPub

// save last state
class SwitchSub: public Listener {
  public:
    SwitchSub(): Listener(ListenerType::OUTPUT_BINARY) {}

    void initListener() override {
      EspNowMQ.on("SWITCH", [&](const uint8_t* data, uint8_t size){
        memcpy(&rx_Msg, data, size);
        //LOGIC HERE
        Serial.println(rx_Msg.data.name);
        if (rx_Msg.data.name == "SWITCH1") {
          VAL1 = rx_Msg.data.toInt();
          digitalWrite(OUTPUT_PIN_1, VAL1);
        } else if (rx_Msg.data.name == "SWITCH2") {
          VAL2 = rx_Msg.data.toInt();
          digitalWrite(OUTPUT_PIN_2, VAL2);
        }
        Serial.print(OUTPUT_PIN_1); Serial.print(" "); Serial.println(VAL1);
        Serial.print(OUTPUT_PIN_2); Serial.print(" "); Serial.println(VAL2);
      });
    }

    void loop() override {
      if (digitalRead(INPUT_PIN_1) == LOW) {
        VAL1 = !VAL1;
        bool sent = EspNowMQ.send("SWITCH", masterMac, "SWITCH1", &VAL1);
        if (sent) {
          Serial.println("Send Success");
        } else {
          Serial.println("Send Failed");
        }

        while (digitalRead(INPUT_PIN_1) == LOW) // wait for button release
          ;
      }

      if (digitalRead(INPUT_PIN_2) == LOW) {
        VAL2 = !VAL2;
        bool sent = EspNowMQ.send("SWITCH", masterMac, "SWITCH2", &VAL2);
        if (sent) {
          Serial.println("Send Success");
        } else {
          Serial.println("Send Failed");
        }

        while (digitalRead(INPUT_PIN_2) == LOW) // wait for button release
          ;
      }

      // Serial.print(OUTPUT_PIN_1); Serial.print(" "); Serial.println(VAL1);
      // Serial.print(OUTPUT_PIN_2); Serial.print(" "); Serial.println(VAL2);
    }; 

    protected: 
      long VAL1 = 0;
      long VAL2 = 0;
}; // end SwitchSub

}