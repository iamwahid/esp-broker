#include "../../core/listener.h"

namespace Modules {

class BlinkPub: public Listener {
  public:
  BlinkPub(): Listener(ListenerType::INPUT_BINARY) {}
  void loop() override {
    if (digitalRead(DEFAULT_BUTTON) == LOW) { // button is pressed
      bool sent = EspNowMQ.send("BLINK", masterMac, "BLINK", (int *)1000);
      if (sent) {
        Serial.println("Send Success");
      } else {
        Serial.println("Send Failed");
      }

      while (digitalRead(DEFAULT_BUTTON) == LOW) // wait for button release
        ;
    }
    delay(200);
  }
}; // end BlinkPub

class BlinkSub: public Listener {
  public:
    BlinkSub(): Listener(ListenerType::OUTPUT_ANALOG) {}

    //BLINK BUILTIN LED
    static void LED_BLINK() {
      digitalWrite(OUTPUT_PIN_1, !digitalRead(OUTPUT_PIN_1));
    }

    void stopBlink() {
      LED_BLINK();
      if (D_isBlinking) {
        D_blinkTimer.detach();
        D_isBlinking = false;
      }
    }

    void startBlink(uint32_t time = 1000) {
      D_blinkTimer.attach_ms(time, LED_BLINK);
      D_isBlinking = true;
    }

    void toggleBlink(uint32_t time = 1000) {
      if (D_isBlinking)
        stopBlink();
      else
        startBlink(time);
    }
    //BLINK BUILTIN LED END

    void initListener() override {
      EspNowMQ.on("BLINK", [&](const uint8_t* data, uint8_t size){
        memcpy(&rx_Msg, data, size);

        //LOGIC HERE
        if (rx_Msg.data.toInt() <= 0) {
          toggleBlink(200);
        } else if (rx_Msg.data.toInt() > 0 && rx_Msg.data.toInt() <= 500) {
          toggleBlink(1000);
        } else if (rx_Msg.data.toInt() > 500 && rx_Msg.data.toInt() <= 1000) {
          toggleBlink(2000);
        }
      });
    }

  protected:
    int D_ledState = HIGH;
    Ticker D_blinkTimer;
    bool D_isBlinking = false;
}; // end BlinkSub


}