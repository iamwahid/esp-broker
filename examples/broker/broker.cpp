#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#define ESPBROKER_AS_BROKER 1

#include "espbroker.h"

unsigned long interval = 5000;
unsigned long prevTime = 5000;
unsigned long currentTime;


void setup()
{
  Serial.begin(115200);
  Serial.println();

  Broker::begin();

  Serial.print("MAC address of this node is ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("Press the button to send a message");
}

void
loop()
{
  currentTime = millis();
  if ((unsigned long)currentTime - prevTime >= interval) {
    prevTime = currentTime;
  }
  delay(10);
}
