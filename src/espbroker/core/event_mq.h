#pragma once

#include "defines.h"
#include "objects.h"
#include "espnow_mq.h"
#include <Ticker.h>
#include <set>
#include <vector>

struct LogicObject {
  bool state = 0;
  String objID = "";
  uint8_t type = LOGIC_TYPE::INVALID;

  std::shared_ptr<DataObject> A_ = nullptr;
  std::shared_ptr<DataObject> B_ = nullptr;
  
  LogicObject() {};
  // LogicObject(String obj_id, long value): state(value), objID(obj_id) {}
  LogicObject(uint8_t type, std::shared_ptr<DataObject> A, std::shared_ptr<DataObject> B): A_(A), B_(B) {this->type = type; }
  virtual ~LogicObject() {};

  virtual bool getState() { return this->state;} // forward value or determine true false
  virtual void updateState() {
    this->state = this->A_->getState() == DATA_STATE::ON; // TODO
  }
  
  uint8_t getType() {
    return this->type;
  }

  virtual void setState(int value) {
    this->state = value;
  }

  void setType(uint8_t type) {
    if (type == LOGIC_TYPE::NO) 
      this->state = 1;
    
    this->type = type;
  }

  void setID(String id) {
    this->objID = id;
  }

  void setA(std::shared_ptr<DataObject> A) {
    this->A_ = A;
  }

  void setB(std::shared_ptr<DataObject> B) {
    this->B_ = B;
  }

  template <typename T>
  void setAConst(T Aconst) {
    this->A_ = std::make_shared<DataObject>();
    this->A_->setName("A");
    this->A_->setValue<T>(Aconst);
  }

  template <typename T>
  void setBConst(T Bconst) {
    this->B_ = std::make_shared<DataObject>();
    this->B_->setName("B");
    this->B_->setValue<T>(Bconst);
  }
};

struct MathObject : LogicObject {  
  MathObject() {};
  MathObject(uint8_t type) {this->type = type; };
  MathObject(uint8_t type, std::shared_ptr<DataObject> A, std::shared_ptr<DataObject> B): LogicObject(type, A, B) { };
  // MathObject(String obj_id, long value): LogicObject(obj_id, value) {}
  virtual ~MathObject() {};

  bool computeEQ() {
    if (this->A_ == nullptr || this->B_ == nullptr || !this->type) return false;

    this->state = this->A_ == this->B_;
    return this->state;
  }

  bool computeNEQ() {
    if (this->A_ == nullptr || this->B_ == nullptr || !this->type) return false;

    this->state = this->A_ != this->B_;
    return this->state;
  }

  bool computeLES() {
    if (this->A_ == nullptr || this->B_ == nullptr || !this->type) return false;

    this->state = this->A_ < this->B_;
    return this->state;
  }

  bool computeLEQ() {
    if (this->A_ == nullptr || this->B_ == nullptr || !this->type) return false;

    this->state = this->A_ <= this->B_;
    return this->state;
  }

  bool computeGRT() {
    if (this->A_ == nullptr || this->B_ == nullptr || !this->type) return false;

    this->state = this->A_ > this->B_;
    return this->state;
  }

  bool computeGRQ() {
    if (this->A_ == nullptr || this->B_ == nullptr || !this->type) return false;

    this->state = this->A_ >= this->B_;
    return this->state;
  }

  void updateState() override {
    switch ( this->getType() )
        {
            case LOGIC_TYPE::EQ:
              {
                  this->computeEQ();
              }
              break;
            case LOGIC_TYPE::NEQ:
              {
                  this->computeNEQ();
              }
              break;
            case LOGIC_TYPE::LES:
              {
                  this->computeLES();
              }
              break;
            case LOGIC_TYPE::LEQ:
              {
                  this->computeLEQ();
              }
              break;
            case LOGIC_TYPE::GRT:
              {
                  this->computeGRT();
              }
              break;
            case LOGIC_TYPE::GRQ:
              {
                  this->computeGRQ();
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
      this->serialObjs.push_back(logicObj);
    } else if (type == WRAP_TYPE::W_PARALEL) {
      this->paralelObjs.push_back(logicObj);
    }
  }

  bool compute() {
    int val = 0;
    bool pVal = 0;
    bool sVal = 0;
    // loop through vector logic

    for (auto pr : this->paralelObjs) {
      pr->updateState();
      pVal = pVal | pr->getState();
    }

    for (auto sr : this->serialObjs) {
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
  LogicWrapper bindings;
  
  bool enabled = true;

  EventObject() {};
  EventObject(String name): event_name(name) {};
  
  virtual ~EventObject() {};

	virtual bool isEnabled() { return this->enabled; }

  /**
   * Dispatch to all subscribers
   */
  virtual void dispatch() {};

  /**
   * Dispatch to mac address
   */
  virtual void dispatch(uint8_t * mac) {};

  virtual bool hasListener(uint8_t * mac) {return false;};
  virtual bool addListener(WifiNowPeer &listener, ListenerRole role) {return false;};
  virtual bool addListener(int pos, ListenerRole role) {return false;};
  virtual bool removeListener(uint8_t * mac) {return false;};
  virtual WifiNowPeer &getListener(uint8_t * mac) {return *static_cast<WifiNowPeer*>(0);};

  virtual void bindParalel(std::shared_ptr<LogicObject> logicObj) { };

  virtual void bindSerial(std::shared_ptr<LogicObject> logicObj) { }

  /**
   * Insert/inject received data to Event
   * Store received (publisher) data
   */
  virtual void enqueue(const uint8_t* data, uint8_t size) {
    memcpy(&rx_Msg, data, sizeof(rx_Msg));
    run();
  };

  virtual void handler(esp_rc_callback_t &callback) { // custom process
    // callback();
  };

  /**
   * Set result Message
   * 
   * @param uint8_t * mac Destination MAC Address 
   * @param DataObject & data
   */
  virtual void computeResult(uint8_t * mac, DataObject &data) {};
  virtual void forward(uint8_t * mac, DataObject &data) {};

  virtual void run() {
#if defined(ARDUINO_ARCH_ESP32)
  Serial.print(":: Task running on core ");
  Serial.print(xPortGetCoreID());
  Serial.println(" ::    ");
#endif
  }

};

struct EventBrokerObject : EventObject {
  std::map<String, std::shared_ptr<WifiNowPeer>> listenerMap;

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
  
  bool isEqual(WifiNowPeer &peer);
	virtual bool isEnabled() { return this->enabled; }

  bool hasListener(uint8_t * mac) override;
  bool addListener(WifiNowPeer &listener, ListenerRole role) override;
  bool addListener(int pos, ListenerRole role) override;
  bool addPublisher(WifiNowPeer &listener);
  bool addSubscriber(WifiNowPeer &listener);
  bool removeListener(uint8_t * mac) override;
  WifiNowPeer &getListener(uint8_t * mac) override;

  /**
   * update Datalist match mac address  
   * 
   */
  bool updateData(uint8_t * mac, DataObject &data, bool compute = true);


  void bindParalel(std::shared_ptr<LogicObject> logicObj) override {
    this->bindings.addLogic(logicObj, WRAP_TYPE::W_PARALEL);
  };

  void bindSerial(std::shared_ptr<LogicObject> logicObj) override {
    this->bindings.addLogic(logicObj, WRAP_TYPE::W_SERIAL);
  };

  virtual void preProcess() { };
  virtual bool postProcess(WifiNowPeer &peer, MessageObject &msg) { return true; }

  void enqueue(const uint8_t* data, uint8_t size) override;
  void dispatch() override;
  void dispatch(uint8_t * mac) override;
  void run() override;
  void computeResult(uint8_t * mac, DataObject &data) override;
  void forward(uint8_t * mac, DataObject &data) override;
  

  void repeatT(uint32_t time = 1000);
  void onceT(uint32_t time = 1000);
  void pauseT(); 
  void toggleT(uint32_t time = 1000);

  protected:
  static void callRun(EventBrokerObject * evObj);
};

extern std::map<String, std::shared_ptr<EventObject>> eventMap;
extern std::map<String, esp_rc_event_t> eventRCMap;

