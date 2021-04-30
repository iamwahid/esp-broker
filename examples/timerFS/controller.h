#pragma once

#include <vector>
#include <WString.h>
#include <Servo.h>
#include <RTClib.h>

Servo servo;

struct strDateTime
{
  byte hour;
  byte minute;
  byte second;
  int year;
  byte month;
  byte day;
  byte dayofWeek;
  boolean valid;
};

struct FeedingObject {
  strDateTime timestamp;
  String timestr;
  long count;
  bool triggered = false;
  FeedingObject() {};
  FeedingObject(String feed) { // timestamp:count
    this->setTime(feed);
    String cnt = feed.substring(feed.indexOf('.')+1);
    count = cnt.toInt();
  }

  void setTime(String strtime) {
    String tms = strtime.substring(0, strtime.indexOf('.'));
    tms.trim();
    this->timestr = tms;
    timestamp.hour = tms.substring(0, tms.indexOf(':')).toInt();
    timestamp.minute = tms.substring(tms.indexOf(':')+1).toInt();
    timestamp.second = 0;
  }

  void handler() {
    for (int i = 0; i < this->count; i++) {
      servo.write(0);
      delay(3000);
      servo.write(180);
      delay(2000);
    }
    // triggered = true;
  }
};

class Feeder {
  public:
  int feeding_size = 0;
  int count = 1;
  std::vector<FeedingObject> feedingList;
  Feeder() {}

  void begin() {
    servo.attach(2);
    servo.write(180);
  }

  void toFeedingList() {
    feedingList.clear();
    for (String tm : v_timer) {
      feedingList.emplace_back(tm);
    }
    feeding_size = feedingList.size();

    for (FeedingObject fd : feedingList) {
      Serial.print(fd.timestamp.hour);
      Serial.print(":");
      Serial.print(fd.timestamp.minute);
      Serial.print(" -- ");
      Serial.println(fd.count);
    }
  }

  void triggerFeedings(DateTime tm){
    for (FeedingObject fd : feedingList) {
      if (fd.timestamp.hour == tm.hour() && fd.timestamp.minute == tm.minute() && (fd.timestamp.second >= tm.second() && fd.timestamp.second <= tm.second()+5)) {
        Serial.println("triggering");
        fd.handler();
      }
    }
  }

  String json_timers() {
    String json_ = "[";
    for (int i = 0; i < this->feeding_size; i++) {
      json_ += "{\"time\": \"" + feedingList[i].timestr + "\", \"count\": " + String(feedingList[i].count) + "}";
      if (i < this->feeding_size - 1) {
        json_ += ",";
      }
    }
    json_ += "]";

    return json_;
  }
  
  void addTimerStr(String timencount) {
    if (!timencount.equals("") && timencount.indexOf(":")>-1) {
      feedingList.emplace_back(timencount);
    }
    this->feeding_size = feedingList.size();
  }

  void addTimer(String timestp, int count = 3) {
    if (!timestp.equals("")) {
      feedingList.emplace_back(timestp + "." + String(count));
    }
    this->feeding_size = feedingList.size();
  }

  bool hasTimer(String timestp) {
    if (!timestp.equals("")) {
      for (int i = 0; i < this->feeding_size; i++) {
        if (feedingList[i].timestr == timestp) {
          return true;
          break;
        }
      }
    }
    return false;
  }

  void updateTimer(String oldtime, String timestp, int count = 3) {
    if (!oldtime.equals("") && !timestp.equals("")) {
      if (hasTimer(oldtime)) {
        deleteTimer(oldtime);
        addTimer(timestp, count);
      }
    }
    this->feeding_size = feedingList.size();
  }

  void deleteTimer(String timestp) {
    if (!timestp.equals("")) {
      for (int i = 0; i < this->feeding_size; i++) {
        if (feedingList[i].timestr == timestp) {
          feedingList.erase(feedingList.begin() + i);
          // break;
        }
      }
    }
    this->feeding_size = feedingList.size();
  }

  void reload() {
    v_timer.clear();
    for (int i = 0; i < this->feeding_size; i++) {
      v_timer.emplace_back(feedingList[i].timestr + "." + String(feedingList[i].count));
    }
  }



  void feed(int count, bool start = true) {
      this->count = count;
      this->is_feed = start;
  }

  void loop() {
    if (is_feed) {
      is_feed = false;
      for (int i = 0; i < this->count; i++) {
        Serial.println("Feeding....");
        servo.write(0);
        delay(3000);
        servo.write(180);
        delay(2000);
      }
    }
  }

  const int max_count = 100;
  long timer_count = 0;
  String timers; // timestamps
  String feeders; // feed count

  std::vector<String> v_timer;
  std::vector<String> v_feeder;

  int idx = 0;

  String filename = "/timers.csv";
  bool is_feed = false;
} feeder;