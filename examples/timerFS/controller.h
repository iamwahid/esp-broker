#pragma once

#include <vector>
#include <WString.h>
#include <Servo.h>
#include <RTClib.h>


Servo servo;

void delayMs(uint32_t time) {
	uint32_t delayUntil = millis() + time;
	while (millis() < delayUntil) yield();
}

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
  long current_count = 0;
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
    if (this->current_count < this->count) {
      servo.write(0);
      delay(3000);
      servo.write(180);
      delay(2000);
      this->current_count++;
    } else {
      this->triggered = false;
      this->current_count = 0;
    }
  }
};

class Feeder {
  public:
  int feeding_size = 0;
  int count = 1;
  int feed_id;
  int curr_count = 0;
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
    for (int i = 0; i < feeding_size; i++) {
      if (!feedingList[i].triggered && feedingList[i].timestamp.hour == tm.hour() && feedingList[i].timestamp.minute == tm.minute() && (feedingList[i].timestamp.second >= tm.second() && feedingList[i].timestamp.second <= tm.second()+5)) {
        feedingList[i].triggered = true;
        feedingList[i].current_count = 0;
        this->feed_id = i;
        this->count = feedingList[i].count;
        this->curr_count = 0;
        this->is_feed = true;
        break;
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
      if (!hasTimer(timestp))
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
      if (curr_count < count) {
        Serial.println("Feeding....");
        servo.write(0);
        delay(3000);
        servo.write(180);
        delay(2000);
        curr_count++;
      } else {
        is_feed = false;
        curr_count = 0;
        if (this->feed_id > 0) {
          feedingList[this->feed_id].triggered = false;
          feedingList[this->feed_id].current_count = 0;
          this->feed_id = -1;
        }
      }
    }

    // for (int i = 0; i < feeding_size; i++) {
    //   if (feedingList[i].triggered) {
    //     Serial.println("triggering");
    //     feedingList[i].handler();
    //     this->is_feed = true;
    //     this->curr_count = this->count;
    //   }
    // }
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

class Blink {
  public:
  bool isBlinking = false;
  Ticker blinker;
  int PIN;
  Blink(int pin) {
    PIN = pin;
    pinMode(PIN, OUTPUT);
  }
  void LED_BLINK() {
    digitalWrite(PIN, !digitalRead(PIN));
  }

  static void cast(Blink *blink) {
    blink->LED_BLINK();
  }

  void on() {
    digitalWrite(PIN, HIGH);
  }

  void off() {
    digitalWrite(PIN, LOW);
  }

  void stop() {
    LED_BLINK();
    if (isBlinking) {
      blinker.detach();
      isBlinking = false;
    }
  }

  void start(uint32_t time = 250) {
    blinker.attach_ms(time, cast, this);
    isBlinking = true;
  }

  void toggle(uint32_t time = 250) {
    if (isBlinking)
      stop();
    else
      start(time);
  }
};