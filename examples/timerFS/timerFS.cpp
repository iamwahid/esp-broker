#define ESPBROKER_AS_LISTENER 1

#include <espbroker.h>
#include <vector>
#include <map>

#include "clock_util.h"

#include <ESPAsyncWebServer.h>
#include <StringArray.h>

#include <ArduinoJson.h>

#include "controller.h"
#include "mqtt.h"

#include "helpers.h"
#include "config.h"

unsigned long cTime;
unsigned long pTime = 1000;
unsigned long interval = 1000;

int trigPin  = D6;
int echoPin  = D5;

long duration;
int jarak;

bool is_save = false;
bool need_restart = false;

String _secretkey;
String _device_ID;

AsyncWebServer server(80); // Create AsyncWebServer object on port 80

void getJarak() {
  // Clears the trigPin
  digitalWrite((uint8_t) trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite((uint8_t) trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite((uint8_t) trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn((uint8_t) echoPin, HIGH);
  // Calculating the jarak
  jarak = duration * 0.034 / 2;
  Serial.println("Jarak " + String(jarak) + " cm");
}

void serverInit() {
  // Route for root / web page
  server.on("/", HTTP_GET, [&](AsyncWebServerRequest * request) {
    Serial.println("Timers");
    request->send_P(200, "application/json", feeder.json_timers().c_str());
  });

  server.on("/timer/add", HTTP_POST, [](AsyncWebServerRequest *request){
      String timestp;
      String count;
      String _secret;
      if (request->hasParam("time", true)) {
          timestp = request->getParam("time", true)->value();
          timestp.trim();
      } else {
          timestp = "";
      }
      if (request->hasParam("count", true)) {
          count = request->getParam("count", true)->value();
          count.trim();
      } else {
          count = "3";
      }

      if (request->hasParam("secret_key", true)) {
          _secret = request->getParam("secret_key", true)->value();
          if (_secretkey.equals(_secret)) {
            feeder.addTimer(timestp, count.toInt());

            is_save = true;
            request->send(200, "text/plain", "Hello, POST: " + timestp + "." + count);
          }
      }

      request->send(403, "text/plain", "Forbidden");

  });

  server.on("/timer/edit", HTTP_POST, [](AsyncWebServerRequest *request){
      String timestp;
      String newtimestp;
      String count;
      String _secret;
      if (request->hasParam("old_time", true)) {
          timestp = request->getParam("old_time", true)->value();
          timestp.trim();
      } else {
          timestp = "";
      }

      if (request->hasParam("time", true)) {
          newtimestp = request->getParam("time", true)->value();
          newtimestp.trim();
      } else {
          newtimestp = "";
      }

      if (request->hasParam("count", true)) {
          count = request->getParam("count", true)->value();
          count.trim();
      } else {
          count = "3";
      }

      if (request->hasParam("secret_key", true)) {
          _secret = request->getParam("secret_key", true)->value();
          feeder.updateTimer(timestp, newtimestp, count.toInt());

          is_save = true;
          request->send(200, "text/plain", "Hello, POST: " + timestp + "." + count);
      }

      request->send(403, "text/plain", "Forbidden");

  });

  server.on("/timer/delete", HTTP_POST, [](AsyncWebServerRequest *request){
      String timestp;
      String count;
      String _secret;
      if (request->hasParam("time", true)) {
          timestp = request->getParam("time", true)->value();
          timestp.trim();
      } else {
          timestp = "";
      }

      if (request->hasParam("secret_key", true)) {
          _secret = request->getParam("secret_key", true)->value();
          feeder.deleteTimer(timestp);

          is_save = true;
          request->send(200, "text/plain", "Hello, POST: " + timestp + "." + count);
      }

      request->send(403, "text/plain", "Forbidden");

  });

  server.on("/status_pakan", HTTP_GET, [](AsyncWebServerRequest *request){
    String json_pakan = "{ \"status\": \"" + pakanStatus(jarak) + "\", \"jarak\": "+ String(jarak) +"}";
    request->send(200, "application/json", json_pakan.c_str());
  });

  server.on("/beri_pakan", HTTP_GET, [](AsyncWebServerRequest *request){
    String _count;
    if (request->hasParam("count")) {
      _count = request->getParam("count")->value();
    }
    request->send(200, "text/plain", "OK");
    feeder.feed(_count.toInt());
  });

  server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request){
    String SSIDname;
    String SSIDpass;
    String APpass;
    String _secret;
    bool _registered = false;

    if (my_preference.isRegistered()) {
        _registered = true;
        if (request->hasParam("secret_key", true)) {
          _secret = request->getParam("secret_key", true)->value();
        }
    }

    if (_registered && _secretkey.equals(_secret)) {
      if (request->hasParam("ssid", true)) {
          SSIDname = request->getParam("ssid", true)->value();
          if (!SSIDname.equals("")) my_preference.setSSID(SSIDname);
      }

      if (request->hasParam("pass", true)) {
          SSIDpass = request->getParam("pass", true)->value();
          my_preference.setPass(SSIDpass);
      }

      if (request->hasParam("ap_pass", true)) {
          APpass = request->getParam("ap_pass", true)->value();
          my_preference.setAPPass(APpass);
      }
      
      need_restart = true;
      request->send(200, "text/plain", "OK");
    }

    request->send(403, "text/plain", "Forbiddden");
  });

  server.on("/register", HTTP_POST, [](AsyncWebServerRequest *request){
    String _deviceid;
    
    if (request->hasParam("device_id", true)) {
        _deviceid = request->getParam("device_id", true)->value();
    }

    if (request->hasParam("secret_key", true)) {
        _secretkey = request->getParam("secret_key", true)->value();
    }

    if (!my_preference.isRegistered()) {
      if (_deviceid.equals(_device_ID) && _secretkey.length() >= 6) {
        my_preference.setSecretKey(_secretkey);
        request->send(200, "text/plain", "OK");
      }
    }

    request->send(400, "text/plain", "Bad Request");
  });

  server.on("/device", HTTP_POST, [](AsyncWebServerRequest *request){
    String _deviceid;
    String _master;
    String _master_key = "12348765";
    
    if (request->hasParam("device_id", true)) {
        _deviceid = request->getParam("device_id", true)->value();
    }

    if (request->hasParam("master_key", true)) {
        _master = request->getParam("master_key", true)->value();
    }

    if (_master.equals(_master_key) && _deviceid.length() > 6) {
      my_preference.setDeviceID(_deviceid);
      request->send(200, "text/plain", "OK");
    }

    request->send(400, "text/plain", "Bad Request");
  });

  // Start server
  server.begin();
}

bool wifiConnect() {
  // if (!WLAN_ENABLED) return true;
  
  // WiFi.persistent(false);
  // WiFi.softAPdisconnect();
  // WiFi.disconnect(true);
  delay(2000);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(my_preference.getAPName().c_str(), my_preference.getAPPass().c_str());
  String ssid_ = my_preference.getSSID();
  ssid_ = sanitize_string_allowlist(ssid_.c_str(), HOSTNAME_CHARACTER_ALLOWLIST).c_str();
  
  if (!ssid_.equals("") && ssid_.length() > 2) {
    WiFi.begin(my_preference.getSSID().c_str(), my_preference.getPass().c_str());
  } else {
    WiFi.begin(my_preference.getSSID(false).c_str(), my_preference.getPass(false).c_str());
  }
  int c = 0;

  Serial.println("Waiting for Wifi to connect");  
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      syncTime();
      return true; 
    } 
    delay(1000);
//    Serial.print(WiFi.status());
    Serial.print(".");
    c++;
  }

  Serial.println("");
  Serial.println("Connect timed out");
  WiFi.disconnect();
  return false;
}

void mqttCallback(char* topic, byte* message, unsigned int length) {
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

  JsonObject& root = jsonBuffer.parseObject(messageTemp.c_str());

  String _secret = root["secret_key"];

  if (_secretkey.equals(_secret)) {
    // if (String(topic) == _device_ID + "/feeder/controller" && root.success()) {
    //   String action = root["action"];
    //   if (action.equals("ADD")) {
    //     String timestp = root["timestp"];
    //     long count = root["count"];
    //     feeder.addTimer(timestp, count);
    //     is_save = true;
    //   } else if (action.equals("EDIT")) {
    //     String timestp = root["timestp"];
    //     String newtimestp = root["newtimestp"];
    //     long count = root["count"];
    //     feeder.updateTimer(timestp, newtimestp, count);
    //     is_save = true;
    //   } else if (action.equals("DELETE")) {
    //     String timestp = root["timestp"];
    //     feeder.deleteTimer(timestp);
    //     is_save = true;
    //   } else if (action.equals("FEED")) {
    //     long count = root["count"];
    //     feeder.feed(count);
    //   }
    // } else 
    if (String(topic).equals( _device_ID + "/feeder/config") ) {
      String _ssid = root["ssid"];
      String _pass = root["pass"];
      String _ap_pass = root["ap_pass"];
      if (!_ssid.equals("")) my_preference.setSSID(_ssid);
      my_preference.setPass(_pass);
      my_preference.setAPPass(_ap_pass);
      need_restart = true;
    } else if (String(topic).equals( _device_ID + "/feeder/beri_pakan") ) {
      feeder.feed(messageTemp.toInt());
    } else if (String(topic).equals( _device_ID + "/feeder/timer/add") ) {
      String timestp = root["time"];
      long count = root["count"];
      feeder.addTimer(timestp, count);
      is_save = true;
    } else if (String(topic).equals( _device_ID + "/feeder/timer/edit") ) {
      String timestp = root["time"];
      String old_time = root["old_time"];
      long count = root["count"];
      feeder.updateTimer(old_time, timestp, count);
      is_save = true;
    } else if (String(topic).equals( _device_ID + "/feeder/timer/delete") ) {
      String timestp = root["time"];
      feeder.deleteTimer(timestp);
      is_save = true;
    }
  }
}

void setup() {
  my_preference.begin();
  FSys.begin();
  Serial.begin(9600);
  pinMode(0, INPUT_PULLUP);
  feeder.begin();
  feeder.v_timer = FSys.loadArray(feeder.filename);
  feeder.toFeedingList();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  startRTC();
  _device_ID = my_preference.getDeviceID();
  _secretkey = my_preference.getSecretKey();

  if (wifiConnect()) {
    mqttc_client.setCallback(mqttCallback);
    mqttc_client.begin();
  }
  serverInit();

}

void saveConfig() {
  feeder.reload();

  if (FSys.saveArray(feeder.v_timer, feeder.filename)) {
    Serial.println("Saved");
  }

}

void loop() {
  cTime = millis();
  if ((unsigned long) cTime - pTime > interval) {
    pTime = cTime;
    // feeder.triggerFeedings(getNTPTime());
    feeder.triggerFeedings(RTC.now());
    getTime();
    getJarak();

    if (feeder.is_feed) {
      mqttc_client.publish(_device_ID + "/feeder/status", "OPEN");
    } else {
      mqttc_client.publish(_device_ID + "/feeder/status", "CLOSE");
    }

    mqttc_client.publish(_device_ID + "/feeder/time", datetime);
    mqttc_client.publish(_device_ID + "/feeder/jarak", String(jarak) + " cm");
    mqttc_client.publish(_device_ID + "/feeder/pakan", pakanStatus(jarak));
    mqttc_client.publish(_device_ID + "/feeder/timers", feeder.json_timers());
    
    if (is_save) {
      is_save= false;
      saveConfig();
    }
    
    if (need_restart) {
      need_restart = false;
      ESP.reset();
    }
    feeder.loop();
  }

  mqttc_client.loop();

  if (digitalRead(0) == LOW) {
    saveConfig();

    delay(5000);
    if (digitalRead(0) == LOW) {
      Serial.println("Reseting...");
      FSys.deleteFile(feeder.filename);
      my_preference.reset();
      delay(1000);
      ESP.restart();
    }
  }

  if (Serial.available() > 1) {
    String line = Serial.readStringUntil('\n');
    Serial.print("USER: ");
    Serial.println(line);
    line.trim();
    feeder.addTimerStr(line);
  }
}
