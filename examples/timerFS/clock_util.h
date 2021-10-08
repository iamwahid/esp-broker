#pragma once
#include <RTClib.h>
#include <Wire.h>

#include <NTPClient.h> //NTPClient by Arduino
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <functional>


#define NTP_SERVER "0.us.pool.ntp.org"

//GMT Time Zone with sign
#define GMT_TIME_ZONE 7

//Force RTC update and store on EEPROM
//change this to a random number between 0-255 to force time update
#define FORCE_RTC_UPDATE 8

RTC_DS3231 RTC;

WiFiUDP ntpUDP;

uint32_t lastTime;


// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
NTPClient timeClient(ntpUDP, NTP_SERVER, GMT_TIME_ZONE * 3600 , 60000);
int timeUpdated = 2;

char datetime[30];

bool RTCbegun = false;
std::function<void(void)> _onTimeSync = nullptr;
std::function<void(void)> _onTimeSynced = nullptr;


void startRTC() {
  EEPROM.begin(4);
  if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  RTCbegun = true;
}

void onTimeSync(std::function<void(void)> callback) {
  _onTimeSync = callback;
}

void onTimeSynced(std::function<void(void)> callback) {
  _onTimeSynced = callback;
}

void syncTime(bool force = false) {
  if (_onTimeSync) {
    _onTimeSync();
  }
  // if (! RTC.begin()) {
  //   Serial.println("Couldn't find RTC");
  //   while (1);
  // }
  
//    if (RTC.lostPower()) {
//      Serial.println("RTC lost power, lets set the time!");
//      // following line sets the RTC to the date &amp; time this sketch was compiled
//      RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
//      // This line sets the RTC with an explicit date &amp; time, for example to set
//      // January 21, 2014 at 3am you would call:
//      // RTC.adjust(DateTime(2014, 1, 21, 3, 0, 0));
//      //  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Time and date is expanded to date and time on your computer at compiletime
//      //  RTC.adjust(DateTime(12, 2, 2021, 3, 0, 0));
//      //  RTC.setDate(12, 2, 2021);
//    }
  byte addvalue = EEPROM.read(timeUpdated);
  Serial.print("EEPROM: ");
  Serial.print(addvalue);
  Serial.print(" == ");
  Serial.print(FORCE_RTC_UPDATE);
  Serial.println(" ?");
  if ((addvalue != FORCE_RTC_UPDATE) || RTC.lostPower() || force) {
    //if(true == false){
    //time hasn' it been setup
    if (RTC.lostPower()) {
      Serial.println("Lost Power...");
    }

    if (force) {
      Serial.println("Forcing Time Update");
    }
    
    timeClient.begin();
    timeClient.update();
    long actualTime = timeClient.getEpochTime();
    Serial.print("Internet Epoch Time: ");
    Serial.println(actualTime);
    RTC.adjust(DateTime(actualTime));
    
    Serial.println("Updating EEPROM..");
    EEPROM.write(timeUpdated, FORCE_RTC_UPDATE);
    EEPROM.commit();

  } else {
    Serial.println("Time has been updated before...EEPROM CHECKED");
    Serial.print("EEPROM: ");
    Serial.print(addvalue);
    Serial.print(" = ");
    Serial.print(FORCE_RTC_UPDATE);
    Serial.println("!");
  }

  if (_onTimeSynced) {
    _onTimeSynced();
  }
}

DateTime getNTPTime() {
  return DateTime(timeClient.getEpochTime());
}

void getTime() {
  DateTime RTCnow = RTC.now();
  DateTime NTPnow = getNTPTime();
  DateTime now = RTCbegun ? RTCnow : NTPnow;
  uint32_t rtcunix = RTCnow.unixtime();
  uint32_t ntpunix = NTPnow.unixtime();

  Serial.print(rtcunix);
  Serial.print(" == ");
  Serial.println(ntpunix);

  if (lastTime == rtcunix || (rtcunix < (ntpunix - 10)) ) {
    syncTime(true);
  }
  sprintf( datetime, "%02d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second() );
  Serial.println(datetime);
  lastTime = rtcunix;
// calculate a date which is 7 days and 30 seconds into the future
//    DateTime future (now + TimeSpan(7, 12, 30, 6));
//  
//    Serial.print(" now + 7d + 30s: ");
//    Serial.print(future.year(), DEC);
//    Serial.print('/');
//    Serial.print(future.month(), DEC);
//    Serial.print('/');
//    Serial.print(future.day(), DEC);
//    Serial.print(' ');
//    Serial.print(future.hour(), DEC);
//    Serial.print(':');
//    Serial.print(future.minute(), DEC);
//    Serial.print(':');
//    Serial.print(future.second(), DEC);
//    Serial.println();
//  
//    Serial.println();
}