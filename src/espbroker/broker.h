#ifndef _EVENT_BROKER_H_
#define _EVENT_BROKER_H_

#include <Ticker.h>
#include "system/EventMQ.h"
#include "config.h"
#include "event.h"


TaskHandle_t taskBroker;


namespace Broker {
  bool WLAN_ENABLED = true; // cfg
  bool CORE0 = true; // cfg
  const char * DSSID = "mbayar0"; //cfg
  const char * DPASS = "qwertyuu"; //cfg

  // pair with type, event(s), role, eventObject cfg
  uint8_t listeners[6][6] = { //cfg
    {0x86, 0x0D, 0x8E, 0x86, 0xD8, 0xF4}, // 86:0D:8E:86:D8:F4 Subscriber
    {0xA4, 0xCF, 0x12, 0x75, 0x09, 0xC5},
    {0x52, 0x02, 0x91, 0x4D, 0xF0, 0x65}, // 52:02:91:4D:F0:65 Publisher
    {0xB6, 0xE6, 0x2D, 0x67, 0xF5, 0x31}, // esp01 B6:E6:2D:67:F5:31
    {0xB6, 0xE6, 0x2D, 0x27, 0x16, 0x3A},
    {0xF4, 0xCF, 0xA2, 0x9A, 0x0C, 0xB1}, // espcam
  };

  // cfg eventObject state(s)

  void initListeners() {
    EspNowMQ.updatePeer(EspNowMQ.peerListeners, listeners[0], "SWITCH");
    EspNowMQ.updatePeer(EspNowMQ.peerListeners, listeners[0], ListenerType::OUTPUT_BINARY);

    EspNowMQ.updatePeer(EspNowMQ.peerListeners, listeners[2], "SWITCH");
    EspNowMQ.updatePeer(EspNowMQ.peerListeners, listeners[2], ListenerType::INPUT_BINARY);

    //example
    std::shared_ptr<EventObject> swit = std::make_shared<Events::BlinkEvent>();
    EspNowMQ.on("SWITCH", swit);
    EspNowMQ.getEvent("SWITCH").addListener(EspNowMQ.indexOf(listeners[0]), ListenerRole::SUB);
    EspNowMQ.getEvent("SWITCH").addListener(EspNowMQ.indexOf(listeners[2]), ListenerRole::PUB);

    std::shared_ptr<LogicObject> logicObj = std::make_shared<MathObject>();

    logicObj->setType(LOGIC_TYPE::EQ);
    logicObj->setA(EspNowMQ.getEvent("SWITCH").getListener(listeners[2]).data);
    logicObj->setB(EspNowMQ.getEvent("SWITCH").getListener(listeners[0]).data);
    
    EspNowMQ.getEvent("SWITCH").addParalel(logicObj);
    
  }

  bool wifiConnect() {
    if (!WLAN_ENABLED) return true;
    
    WiFi.begin(DSSID, DPASS);
    int c = 0;

    Serial.println("Waiting for Wifi to connect");  
    while ( c < 20 ) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true; 
      } 
      delay(1000);
  //    Serial.print(WiFi.status());
      Serial.print(".");    
      c++;
    }
    
    Serial.println("");
    Serial.println("Connect timed out");
    return WiFi.disconnect();
  }

  void startBroker() {
    if (wifiConnect()) {
      bool ok = EspNowMQ.begin(NET_ROLE::BROKER, DSSID);
      if (!ok) {
        Serial.println("failed");
        ESP.restart();
      }

      for (int i = 0; i < 5; i++) {
        if (listeners[i]) {
          EspNowMQ.addPeer(listeners[i], 0, nullptr, ESP_IF_WIFI_STA);
        }
      }

      EspNowMQ.refreshPeers();
      initListeners();
    }
  }

  void vTaskBroker( void * pvParameters ){
    Serial.print("Task running on core ");
    Serial.println(xPortGetCoreID());

    startBroker();
    //=======DO NOT DELETE============/
    // for(;;)
    // {
        
    // }
    vTaskDelete(NULL);
    //================================/
  }

  void begin() {
    // WiFi.persistent(false);
    // WiFi.mode(WIFI_AP_STA);
    // WiFi.softAP("ESPNOW", nullptr, 3);
    // WiFi.softAPdisconnect(false);
    
    if (CORE0) {
      xTaskCreatePinnedToCore(
          vTaskBroker,   /* Task function. */
          "TaskBroker",     /* name of task. */
          10000,       /* Stack size of task */
          NULL,        /* parameter of the task */
          1,           /* priority of the task */
          &taskBroker,      /* Task handle to keep track of created task */
          0);          /* pin task to core 0 */                  
      delay(500);
      
    } else {
      startBroker();
    }
    
    
  }

  void loop() {
    if (digitalRead(DEFAULT_BUTTON) == LOW) { // button is pressed

      while (digitalRead(DEFAULT_BUTTON) == LOW) // wait for button release
        ;
    }

    delay(200);
  }
};

#endif