#ifndef EVENT_MQ_H
#define EVENT_MQ_H

#include "defines.h"
#include "objects.h"
#include "EspNowMQ.h"
#include <Ticker.h>
#include <set>
#include <vector>

struct LogicObject {
  bool state = 0;
  String objID = "";
  uint8_t type_ = LOGIC_TYPE::INVALID;

  std::shared_ptr<DataObject> A_ = nullptr;
  std::shared_ptr<DataObject> B_ = nullptr;
  
  LogicObject() {};
  // LogicObject(String obj_id, long value): state(value), objID(obj_id) {}
  LogicObject(uint8_t type, std::shared_ptr<DataObject> A, std::shared_ptr<DataObject> B): A_(A), B_(B) {type_ = type; }
  virtual ~LogicObject() {};

  virtual bool getState() { return state;}
  virtual void updateState() {
    state = A_->getValue<bool>();
  }
  
  uint8_t getType() {
    return type_;
  }

  virtual void setState(int value) {
    this->state = value;
  }

  void setType(uint8_t type) {
    if (type == LOGIC_TYPE::NO) 
      state = 1;
    
    type_ = type;
  }

  void setID(String id) {
    objID = id;
  }

  void setA(std::shared_ptr<DataObject> A) {
    A_ = A;
  }

  void setB(std::shared_ptr<DataObject> B) {
    B_ = B;
  }

  // bool operator!() {
  //   return type_ == LOGIC_TYPE::INVALID && !state;
  // }

  // bool operator<(const LogicObject& logic) {
  //   return state < logic.state;
  // }

  // bool operator>(const LogicObject& logic) {
  //   return state > logic.state;
  // }

  // bool operator==(const LogicObject& logic) {
  //   return (state == logic.state) && type_ == logic.type_;
  // }

  // bool operator<=(const LogicObject& logic) {
  //   return (*this == logic) || (*this < logic);
  // }

  // bool operator>=(const LogicObject& logic) {
  //   return (*this == logic) || (*this > logic);
  // }

  // bool operator!=(const LogicObject& logic) {
  //   return !(*this == logic);
  // }

  // bool operator&&(const LogicObject& logic) {
  //   return (*this == logic);
  // }

  // bool operator||(const LogicObject& logic) {
  //   return (state || logic.state);
  // }

  // void operator=(const LogicObject& logic) {
  //   return this->w;
  // }
};

struct MathObject : LogicObject {  
  MathObject() {};
  MathObject(uint8_t type) {type_ = type; };
  MathObject(uint8_t type, std::shared_ptr<DataObject> A, std::shared_ptr<DataObject> B): LogicObject(type, A, B) { };
  // MathObject(String obj_id, long value): LogicObject(obj_id, value) {}
  virtual ~MathObject() {};

  bool computeEQ() {
    if (A_ == nullptr || B_ == nullptr || !type_) return false;

    state = A_ == B_;
    return state;
  }

  bool computeNEQ() {
    if (A_ == nullptr || B_ == nullptr || !type_) return false;

    state = A_ != B_;
    return state;
  }

  bool computeLES() {
    if (A_ == nullptr || B_ == nullptr || !type_) return false;

    state = A_ < B_;
    return state;
  }

  bool computeLEQ() {
    if (A_ == nullptr || B_ == nullptr || !type_) return false;

    state = A_ <= B_;
    return state;
  }

  bool computeGRT() {
    if (A_ == nullptr || B_ == nullptr || !type_) return false;

    state = A_ > B_;
    return state;
  }

  bool computeGRQ() {
    if (A_ == nullptr || B_ == nullptr || !type_) return false;

    state = A_ >= B_;
    return state;
  }

  void updateState() override {
    switch ( getType() )
        {
            case LOGIC_TYPE::EQ:
              {
                  computeEQ();
              }
              break;
            case LOGIC_TYPE::NEQ:
              {
                  computeNEQ();
              }
              break;
            case LOGIC_TYPE::LES:
              {
                  computeLES();
              }
              break;
            case LOGIC_TYPE::LEQ:
              {
                  computeLEQ();
              }
              break;
            case LOGIC_TYPE::GRT:
              {
                  computeGRT();
              }
              break;
            case LOGIC_TYPE::GRQ:
              {
                  computeGRQ();
              }
              break;
            default:
              break;
        }
  }
};

struct LogicWrapper {
  std::vector<std::vector<std::shared_ptr<LogicObject>>> conditions;
  std::vector<std::shared_ptr<LogicObject>> paralelObjs, serialObjs;

  LogicWrapper() {}
  virtual ~LogicWrapper() {}

  void addLogic(std::shared_ptr<LogicObject> logicObj, uint8_t type) { // not valid
    if (type == WRAP_TYPE::W_SERIAL) {
      serialObjs.push_back(logicObj);
    } else if (type == WRAP_TYPE::W_PARALEL) {
      paralelObjs.push_back(logicObj);
    }
  }

  bool compute() {
    int val = 0;
    bool pVal = 0;
    bool sVal = 0;
    // loop through vector logic

    for (auto pr : paralelObjs) {
      pr->updateState();
      pVal = pVal | pr->getState();
    }

    for (auto sr : serialObjs) {
      sr->updateState();
      sVal = sVal & sr->getState();
    }

    val = pVal || sVal;
    return val;
  }
};

struct EventObject {
  String event_name;

  MessageObject rx_Msg;
  LogicWrapper logicals;
  
  bool enabled = true;

  EspNowMQPeerInfo plain;

  EventObject() {};
  EventObject(String name): event_name(name) {};
  
  virtual ~EventObject() {};

	virtual bool isEnabled() { return enabled; }

  virtual bool hasListener(uint8_t * mac) {return false;};
  virtual bool addListener(EspNowMQPeerInfo &listener, ListenerRole role) {return false;};
  virtual bool addListener(int pos, ListenerRole role) {return false;};
  virtual bool removeListener(uint8_t * mac) {return false;};
  virtual EspNowMQPeerInfo &getListener(uint8_t * mac) {
    return plain;
  };

  virtual bool logicalResult() { return true; };

  virtual void addParalel(std::shared_ptr<LogicObject> logicObj) { };

  virtual void addSerial(std::shared_ptr<LogicObject> logicObj) { }

  virtual void process(const uint8_t* data, uint8_t size) {
    memcpy(&rx_Msg, data, sizeof(rx_Msg));
    run();
  };

  virtual void run() {
#if defined(ARDUINO_ARCH_ESP32)
  Serial.print(":: Task running on core ");
  Serial.print(xPortGetCoreID());
  Serial.println(" ::    ");
#endif
  }

};

struct EventBrokerObject : EventObject {
  std::map<String, std::shared_ptr<EspNowMQPeerInfo>> listenerMap;

  std::set<String> pubSet = {};
  std::set<String> subSet = {};

  MessageObject result;
  int listenerCount = 0;
  const static int MAX_PEERS = 20;

  bool repeating = false;

  uint8_t runType = RUN_TYPE::ONCE;
  Ticker tickr;

  EventBrokerObject(): EventObject() {};
  EventBrokerObject(String name): EventObject(name) {};
  
  virtual ~EventBrokerObject() {};

  bool hasListener(uint8_t * mac) override;
  bool addListener(EspNowMQPeerInfo &listener, ListenerRole role) override;
  bool addListener(int pos, ListenerRole role) override;
  bool addPublisher(EspNowMQPeerInfo &listener);
  bool addSubscriber(EspNowMQPeerInfo &listener);
  bool removeListener(uint8_t * mac) override;
  EspNowMQPeerInfo &getListener(uint8_t * mac) override;

  bool updateData(uint8_t * mac, DataObject &data, bool setResult = true);

  bool isEqual(EspNowMQPeerInfo &peer);

  virtual bool logicalResult() {
    return logicals.compute();
  };

  void addParalel(std::shared_ptr<LogicObject> logicObj) override {
    logicals.addLogic(logicObj, WRAP_TYPE::W_PARALEL);
  };

  void addSerial(std::shared_ptr<LogicObject> logicObj) override {
    logicals.addLogic(logicObj, WRAP_TYPE::W_SERIAL);
  };

  virtual void preProcess() { };

  virtual bool postProcess(EspNowMQPeerInfo &peer, MessageObject &msg) {
		return true;
	}


	virtual bool isEnabled() { return enabled; }

  void process(const uint8_t* data, uint8_t size) override;
  void run() override;
  
  void repeat(uint32_t time = 1000);

  void once(uint32_t time = 1000);

  void pause(); 

  void retoggle(uint32_t time = 1000);

  private:
  static void sRun(EventBrokerObject * evObj);
};

extern std::map<String, std::shared_ptr<EventObject>> eventMap;
extern std::map<String, esp_rc_event_t> eventRCMap;

#endif