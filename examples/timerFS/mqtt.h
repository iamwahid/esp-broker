#pragma once

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// #include "config.h"

const int MAX_ATTEMP = 2;
int attemp = 0;

class MQTTC {
  public:
  const char* mqtt_server = "178.128.217.111";
  WiFiClient espClient;
  PubSubClient client{espClient};
  long lastMsg = 0;
  char msg[50];
  int value = 0;
  bool callback_setted = false;

  bool begun = false;

  void setCallback(MQTT_CALLBACK_SIGNATURE) {
    callback_setted = true;
    client.setCallback(callback);
  }

  void static callback(char* topic, byte* message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;
    StaticJsonBuffer<250> jsonBuffer;
    
    for (int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
      messageTemp += (char)message[i];
    }
    Serial.println();

    // Feel free to add more if statements to control more GPIOs with MQTT
    JsonObject& root = jsonBuffer.parseObject(messageTemp.c_str());

    // If a message is received on the topic espFeeder/controller, you check if the message is either "on" or "off". 
    // Changes the output state according to the message
    if (String(topic) == "espFeeder/controller" && root.success()) {
      const char * action = root["action"];
      const char * timestp = root["timestp"];
      long count = root["count"];
      Serial.println(action);
      Serial.println(timestp);
      Serial.println(count);
    }
  }

  void publish(String key, String value) {
    client.publish(String("espFeeder/" + key).c_str(), value.c_str());
  }

  void reconnect() {
    // Loop until we're reconnected
    while (!client.connected() && attemp <= MAX_ATTEMP) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect("ESP8266Client")) {
        Serial.println("connected");
        // Subscribe
        client.subscribe("espFeeder/controller");
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
      attemp++;
    }
  }

  void begin() {
    client.setServer(mqtt_server, 1883);
    if (!callback_setted) {
      client.setCallback(callback);
    }
    begun = true;
  }

  void loop() {
    if (begun) {
      if (!client.connected()) {
        reconnect();
      }
      // attemp = 0;
      client.loop();
    }
  }

} mqttc_client;

