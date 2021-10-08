#pragma once

#include <string>
#include <map>
#include <WString.h>
#include <espbroker/core/simple_type.h>
#include <espbroker/core/preferences.h>
#include <espbroker/core/helpers.h>

using namespace espbroker;

const int ENV = 1;

class MyPreference {
  private:
  const char * MASTER_KEY = "12348765";

  long_char_t device_id;
  long_char_t ap_name;
  long_char_t ap_pass;
  long_char_t ssid;
  long_char_t pass;
  long_char_t secret_key;
  long default_feed;

  // long_char_t _device_id = "USW1000001";
  // long_char_t _ap_name = "FEEDR-USW1000001";
  long_char_t _ap_pass = "12348765";
  long_char_t _ssid = "Mitra";
  long_char_t _pass = "12348765";
  long_char_t _secret_key = "";
  long _default_feed = 3;

  long registered = 0;
  std::string ap_name_ = "ap_name_";
  std::string ap_pass_ = "ap_pass_";

  std::string ssid_ = "ssid_";
  std::string pass_ = "pass_";

  std::string device_id_ = "device_id_";
  std::string secret_key_ = "secret_key_";

  std::string default_feed_ = "default_feed_";

  std::string registered_ = "registered_";

  std::map<std::string, ESPPreferenceObject> preferences;

  uint32_t getkey_(std::string name_) {
    return fnv1_hash(sanitize_string_allowlist(to_lowercase_underscore(name_), HOSTNAME_CHARACTER_ALLOWLIST));
  }

  template <typename T> 
  bool load(std::string name_, T &value_) {
    std::map<std::string, ESPPreferenceObject>::iterator pref_item = preferences.find(name_);

    if (pref_item != preferences.end()) {
      Serial.print(name_.c_str());
      Serial.println(" Found");
      if (pref_item->second.load<T>(&value_)) {
        Serial.print(name_.c_str());
        Serial.print(" : ");
        Serial.print(String(value_));
        Serial.println(" Loaded");
        return true;
      }
    } else {
      ESPPreferenceObject prefe = global_preferences.make_preference<T>(getkey_(name_));
      if (prefe.load<T>(&value_)) {
        Serial.print(name_.c_str());
        Serial.print(" : ");
        Serial.print(value_);
        Serial.println(" Loaded");
        preferences.emplace(name_, std::move(prefe));
        return true;
      }
    }

    return false;
  }

  template <typename T>
  bool save(std::string name_,T value_) {
    std::map<std::string, ESPPreferenceObject>::iterator pref_item = preferences.find(name_);

    if (pref_item != preferences.end()) {
      Serial.print(name_.c_str());
      Serial.println(" Found");
      if (pref_item->second.save<T>((T *)value_)) {
        Serial.print(name_.c_str());
        Serial.print(" : ");
        Serial.print(value_);
        Serial.println(" Saved");
        return true;
      }
    } else {
      ESPPreferenceObject prefe = global_preferences.make_preference<T>(getkey_(name_));
      if (prefe.save<T>((T *)value_)) {
        Serial.print(name_.c_str());
        Serial.print(" : ");
        Serial.print(value_);
        Serial.println(" Saved");
        preferences.emplace(name_, std::move(prefe));
        return true;
      }
    }

    return false;
  }

  public:
  String getMasterKey() {
    return MASTER_KEY;
  }

  String getAPName() {
    return "FEEDR-" + this->getDeviceID();
  }

  String getAPPass(bool load_ = true) {
    if (load_) { 
      load(ap_pass_, ap_pass);
      return ap_pass;
    }
    return _ap_pass;
  }

  String getSSID(bool load_ = true) {
    if (load_) { 
      load(ssid_, ssid);
      return ssid;
    }
    return _ssid;
  }

  String getPass(bool load_ = true) {
    if (load_) { 
      load(pass_, pass);
      return pass;
    }
    return _pass;
  }

  String getDeviceID() {
    load(device_id_, device_id);
    return device_id;
  }

  String getSecretKey(bool load_ = true) {
    if (load_) { 
      load(secret_key_, secret_key);
      return secret_key;
    }
    return _secret_key;
  }

  long getDefaultFeed(bool load_ = true) {
    if (load_) { 
      load<long>(default_feed_, default_feed);
      return default_feed;
    }
    return _default_feed;
  }

  bool isRegistered() {
    String _secret = this->getSecretKey();

    return !_secret.equals("") && _secret.length() >= 6;
  }

  // bool setAPName(String value_) {
  //   value_.toCharArray(ap_name, sizeof(long_char_t));
  //   return save<long_char_t>(ap_name_, ap_name);
  // }

  bool setAPPass(String value_) {
    value_.toCharArray(ap_pass, sizeof(long_char_t));
    return save<long_char_t>(ap_pass_, ap_pass);
  }

  bool setSSID(String value_) {
    value_.toCharArray(ssid, sizeof(long_char_t));
    return save<long_char_t>(ssid_, ssid);
  }

  bool setPass(String value_) {
    value_.toCharArray(pass, sizeof(long_char_t));
    return save<long_char_t>(pass_, pass);
  }

  bool setDeviceID(String value_) {
    value_.toCharArray(device_id, sizeof(long_char_t));
    return save<long_char_t>(device_id_, device_id);
  }

  bool setSecretKey(String value_) {
    if (!value_.equals("") && value_.length() < 6) return false; 
    value_.toCharArray(secret_key, sizeof(long_char_t));
    return save<long_char_t>(secret_key_, secret_key);
  }

  bool setDefaultFeed(long value_) {
    default_feed = value_;
    return save<long>(default_feed_, default_feed);
  }

  bool setRegistration(bool valid_ = false) {
    registered = valid_ ? 1 : 0;
    return save<long>(registered_, registered);
  }

  void reset() {
    this->setAPPass(_ap_pass);
    this->setSSID(_ssid);
    this->setPass(_pass);
    this->setSecretKey("");
  }

  void begin() {
    global_preferences.begin();
    // preferences.emplace(ap_name_, std::move(global_preferences.make_preference<long_char_t>(getkey_(ap_name_))) );
    preferences.emplace(ap_pass_, std::move(global_preferences.make_preference<long_char_t>(getkey_(ap_pass_))) );
    preferences.emplace(ssid_, std::move(global_preferences.make_preference<long_char_t>(getkey_(ssid_))) );
    preferences.emplace(pass_, std::move(global_preferences.make_preference<long_char_t>(getkey_(pass_))) );
    preferences.emplace(device_id_, std::move(global_preferences.make_preference<long_char_t>(getkey_(device_id_))) );
    preferences.emplace(secret_key_, std::move(global_preferences.make_preference<long_char_t>(getkey_(secret_key_))) );
    preferences.emplace(registered_, std::move(global_preferences.make_preference<long>(getkey_(registered_))) );
    preferences.emplace(default_feed_, std::move(global_preferences.make_preference<long>(getkey_(default_feed_))) );

    this->getSSID();
    this->getPass();
    this->getAPPass();
    this->getDeviceID();
    this->getSecretKey();
    this->getDefaultFeed();
    this->isRegistered();
  }
  

} my_preference;