#if defined(ARDUINO_ARCH_ESP8266)
#include <c_types.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <esp_wifi.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

#include "EventMQ.h"
#include "objects.h"
#include "helpers.h"
#include <map>
#include <memory>
#include <WString.h>
#include <cstddef>
#include <cstdint>

std::map<String, std::shared_ptr<EventObject>> eventMap;
std::map<String, esp_rc_event_t> eventRCMap;

MessageObject rxMsg;

// LISTENER
bool EventBrokerObject::hasListener(uint8_t * mac) {
  std::map<String, std::shared_ptr<EspNowMQPeerInfo>>::iterator lsiter = listenerMap.find(macStr(mac));
  if (lsiter != listenerMap.end()) {
    DEBUG_PRINT("listener " + macStr(mac) + " exist\n");
    // if (lsiter->second.listenOn == event_name) {
      return true;
    // }
  }
  DEBUG_PRINT("listener " + macStr(mac) + " not exist\n");
  return false;
}

bool EventBrokerObject::addListener(EspNowMQPeerInfo &listener, ListenerRole role) {
  // if (listener.listenOn != event_name) return false;
  // if (listener.channel == 0xFF) return false;
  listener.role = role;
  listener.status = ListenerStatus::ONLINE;

  listenerMap.emplace(macStr(listener.mac), std::make_shared<EspNowMQPeerInfo>(listener));
  listenerCount = listenerMap.size();

  if (listener.role == ListenerRole::PUB) {
     DEBUG_PRINT("new listener " + macStr(listener.mac) + " as Publisher\n");
     pubSet.emplace(macStr(listener.mac));
  } else if (listener.role == ListenerRole::SUB) {
     DEBUG_PRINT("new listener " + macStr(listener.mac) +" as Subscriber\n");
     subSet.emplace(macStr(listener.mac));
  }

  DEBUG_PRINT(String(listenerMap.size()) + " listener\n");
  return true;
}

bool EventBrokerObject::addListener(int pos, ListenerRole role) {
  if (pos<0) return false;
  return addListener(EspNowMQ.peerListeners[pos], role);
}

bool EventBrokerObject::removeListener(uint8_t * mac) {
  if (getListener(mac).role == ListenerRole::PUB) pubSet.erase(macStr(mac));
  else if (getListener(mac).role == ListenerRole::SUB) subSet.erase(macStr(mac));

  listenerCount = listenerMap.erase(macStr(mac));
  DEBUG_PRINT(listenerCount+" listener\n");
  return true;
}

bool EventBrokerObject::addPublisher(EspNowMQPeerInfo &listener) {
  return addListener(listener, ListenerRole::PUB);
}

bool EventBrokerObject::addSubscriber(EspNowMQPeerInfo &listener) {
  return addListener(listener, ListenerRole::SUB);
}

EspNowMQPeerInfo &EventBrokerObject::getListener(uint8_t * mac) {
  return *listenerMap.at(macStr(mac));
};

bool EventBrokerObject::isEqual(EspNowMQPeerInfo &peer) {
  return EspNowMQ.equals(rx_Msg.src, peer.mac, 6);
}


void EventBrokerObject::process(const uint8_t* data, uint8_t size) {
  memcpy(&rx_Msg, data, sizeof(rx_Msg));
  // store from publisher
  updateData(rx_Msg.src, rx_Msg.data, false);
  preProcess();
  run();
};

bool EventBrokerObject::updateData(uint8_t * mac, DataObject &data, bool setResult) {
  if (hasListener(mac)) {
    getListener(mac).data->name = data.name;
    getListener(mac).data->value = data.value;
    // getListener(mac).configs.minVal.setValue<long>(100);
    // getListener(mac).configs.maxVal.setValue<long>(1000);
  }
  if (setResult) {
    result.data.name = data.name;
    // result.data.value = data.value;
    memcpy(result.dst, mac, 6);  
#if defined(ARDUINO_ARCH_ESP8266)
    wifi_get_macaddr(SOFTAP_IF, result.src); // mode
#elif defined(ARDUINO_ARCH_ESP32)
    esp_wifi_get_mac(WIFI_IF_AP, result.src);
#endif

    if (logicalResult()) {
      if (getListener(mac).type == ListenerType::OUTPUT_BINARY) {
        result.data.setBool(getListener(mac).data->configs.onVal.getValue<bool>());
      } else if (getListener(mac).type == ListenerType::OUTPUT_ANALOG) {
        result.data.setFloat(getListener(mac).configs.onVal.toFloat()); // TODO
      }
    } else {
      if (getListener(mac).type == ListenerType::OUTPUT_BINARY) {
        result.data.setBool(getListener(mac).configs.offVal.getValue<bool>());
      } else if (getListener(mac).type == ListenerType::OUTPUT_ANALOG) {
        result.data.setFloat(getListener(mac).configs.offVal.toFloat());
      }
    }
  }
  return true;
}

void EventBrokerObject::sRun(EventBrokerObject * evObj) {
    evObj->run();
}

void EventBrokerObject::repeat(uint32_t time) {
  tickr.attach_ms(time, &EventBrokerObject::sRun, this);
  repeating = true;
  runType = RUN_TYPE::PERIODIC;
}

void EventBrokerObject::once(uint32_t time) {
  tickr.once_ms(time, &EventBrokerObject::sRun, this);
  runType = RUN_TYPE::ONCE;
}

void EventBrokerObject::pause() {
  tickr.detach();
  repeating = false;
}

void EventBrokerObject::retoggle(uint32_t time) {
  if (repeating)
    pause();
  else
    repeat(time);
}

void EventBrokerObject::run() {
#if defined(ARDUINO_ARCH_ESP32)
  Serial.print(":: Task running on core ");
  Serial.print(xPortGetCoreID());
  Serial.println(" ::    ");
#endif

  uint8_t Taddr[6];

  // for (auto mac : pubSet)
  // {
  //   macHex(Taddr, (char *)mac.c_str());
  //   updateData(Taddr, rx_Msg.data);
  // }

  for (auto mac : subSet) {    
    macHex(Taddr, (char *)mac.c_str());
    updateData(Taddr, rx_Msg.data); // update value before forwarded to subscriber
    
    
    if ( postProcess(getListener(Taddr), rx_Msg) && getListener(Taddr).status != ListenerStatus::OFFLINE ) {
      getListener(Taddr).status = ListenerStatus::BUSY;
      if (EspNowMQ.send(Taddr, (uint8_t *)&result, sizeof(result))) {
        DEBUG_PRINT("Send Success\n");
        DEBUG_PRINT(result.data.name + "\n");
      } else {
        DEBUG_PRINT("Send Failed\n");
      }
    }
  }


  // for (auto it = listenerMap.begin(); it != listenerMap.end(); it++) {
  //   // if (it->second.listenOn != rx_Msg.m_event_name) continue;
  //   if (it->second->role != ListenerRole::SUB) continue;
    
  //   // updateData(it->second->mac, rx_Msg.data);
    
  //   if (postProcess(*it->second, rx_Msg)
  //     // && !isEqual(*it->second)
  //     ) {
  //     // send copied and modified messageObject
  //     if (EspNowMQ.send(it->second->mac, (uint8_t *)&result, sizeof(result))) {
  //       DEBUG_PRINT("Send Success\n");
  //       DEBUG_PRINT(result.data.name + "\n");
  //     } else {
  //       DEBUG_PRINT("Send Failed\n");
  //     }
  //   }
  // }
}

void EspNowMQClass::on(String event_name, esp_rc_data_callback_t data_callback, esp_rc_callback_t callback) {
  eventRCMap.emplace(event_name, (esp_rc_event_t){event_name, callback, data_callback});
}

void EspNowMQClass::on(String event_name, std::shared_ptr<EventObject> event_object) {
  eventMap.emplace(event_name, event_object);
}

void EspNowMQClass::on(String event_name) {
  eventMap.emplace(event_name, std::make_shared<EventObject>(event_name));
}

EventObject &EspNowMQClass::getEvent(String event_name) {
  return *eventMap.at(event_name);
}

void EspNowMQClass::recvHandler(const uint8_t *addr, const uint8_t *data, uint8_t size) {
#ifdef MQ_DEBUG_ALL_MSG
	static uint8_t i;
  DEBUG_PRINT("Message from " + macStr((uint8_t *)addr)+"\n");
	Serial.printf(":: MESH_RC RECV %u bytes: ", size);
	for (i = 0; i < size; i++) {
		Serial.write(data[i]);
	}
	Serial.printf(" [ ");
	for (i = 0; i < size; i++) {
		Serial.printf("%02X ", data[i]);
	}
	Serial.println("]");
#endif
  memcpy(&rxMsg, (MessageObject *) data, sizeof(rxMsg));
#ifdef MQ_DEBUG_CONFIRMATION
  MessageObject confirmation;
  confirmation.confirm = true;
  confirmation.m_event_name = rxMsg.m_event_name;
  if (rxMsg.confirm == false) {
    DEBUG_PRINT("Confirm to " + macStr((uint8_t *)addr)+"\n");
    bool result = EspNowMQ.send((uint8_t *)addr, (uint8_t *)&confirmation, sizeof(confirmation));
    
    if (result) {
      DEBUG_PRINT("Confirmation sent\n");
    } else {
      DEBUG_PRINT("Error sending Confirmation\n");
    }
  } else {
    EspNowMQ.duration_c = micros() - EspNowMQ.sendTime;
    DEBUG_PRINT(String(EspNowMQ.duration_c) + " µs\n");
    DEBUG_PRINT("Delivery Confirmed\n");

    if (EspNowMQ.master != NULL && EspNowMQ.equals(addr, EspNowMQ.master, 6)) { 
      // confirmed = true;
    } else if (EspNowMQ.master == NULL) { // in master only
      if (EspNowMQ.getEvent(rxMsg.m_event_name).hasListener((uint8_t *)addr)) {
        EspNowMQ.getEvent(rxMsg.m_event_name).getListener((uint8_t *)addr).status = ListenerStatus::ONLINE;
        DEBUG_PRINT(macStr((uint8_t *)addr) + " ONLINE\n");
      }
    }
    return;
  }
#endif

	if (EspNowMQ.m_rxCb != nullptr) {
    EspNowMQ.m_rxCb((uint8_t *)addr, (uint8_t *)data, size, EspNowMQ.m_rxArg);
  } 

	// Only receives from master if set
	if (addr == NULL || EspNowMQ.master == NULL || EspNowMQ.equals(addr, EspNowMQ.master, 6)) {
		EspNowMQ.received++;
		EspNowMQ.sender = (uint8_t *)addr;

    std::map<String, esp_rc_event_t>::iterator evRCitr = eventRCMap.find(rxMsg.m_event_name);
    if (evRCitr != eventRCMap.end()) {
      if (evRCitr->second.on_cb) evRCitr->second.on_cb();
      if (evRCitr->second.on_data_cb) evRCitr->second.on_data_cb(data, size);
    }

    std::map<String, std::shared_ptr<EventObject>>::iterator eventitr = eventMap.find(rxMsg.m_event_name);
    if (eventitr != eventMap.end()) {
      if (eventitr->second->isEnabled()) eventitr->second->process(data, size);
    }
	} else {
		EspNowMQ.ignored++;
	}
};
