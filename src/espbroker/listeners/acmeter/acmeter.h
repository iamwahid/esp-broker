#include "../base.h"

namespace Modules {

class ACMeterPub: public Listener {
  public:
  String str, sV, sI;
  float vV = 0.0, vI = 0.0;
  ACMeterPub(): Listener(ListenerType::INPUT_ANALOG) {}
  void loop() override {
    if (Serial.available() >= 1) {
      str = Serial.readStringUntil('\n');
      str.trim();
      if (str.indexOf('[')>-1 && str.indexOf(']')>-1) {
        Serial.println(str);
        sV = str.substring(str.indexOf('[')+3, str.indexOf('|'));
        sI = str.substring(str.indexOf('|')+3, str.indexOf(']'));
        vV = sV.toFloat();
        vI = sI.toFloat();
        Serial.print("Voltage: ");
        Serial.println(vV);
        Serial.print("Current: ");
        Serial.println(vI);
        Serial.print("Wattage: ");
        Serial.println(vI*vV);
        bool sent = EspNowMQ.send("ELECTRY", masterMac, "VOLTAGE", &vV); delay(200);
        sent = EspNowMQ.send("ELECTRY", masterMac, "CURRENT", &vI); delay(200);
        float watt = ((float)vI) * ((float)vV);
        sent = EspNowMQ.send("ELECTRY", masterMac, "WATTAGE", &watt); delay(200);
        if (sent) {
        Serial.println("Send Success");
        } else {
        Serial.println("Send Failed");
        }
      }
    }
  }
  
};

}