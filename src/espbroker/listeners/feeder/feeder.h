#include "../../core/listener.h"
#include "Servo.h"

namespace Modules {

class Feeder: public Listener {
  public:
  Feeder(): Listener(ListenerType::INPUT_BINARY) {}
  Servo servo;
  void initListener() override {
    servo.attach(OUTPUT_PIN_1);
    EspNowMQ.on("FEEDER START", [&](const uint8_t* data, uint8_t size){
      servo.write(0);
      delay(3000);
      servo.write(180);
      Serial.println("Feeded");
    });
  }

  void loop() override {
    if (digitalRead(DEFAULT_BUTTON) == LOW) { // button is pressed
      bool sent = EspNowMQ.send("FEEDER", masterMac, "FEEDER", (int *)1000);
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
}; // end Feeder


}