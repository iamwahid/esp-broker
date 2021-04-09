#ifndef __DEFS_H_
#define __DEFS_H_
#include <type_traits>

#include <map>
#include <memory>
#include <WString.h>
#include <cstddef>
#include <cstdint>
#include <helpers.h>

typedef std::function<void(void)> esp_rc_callback_t;
typedef std::function<void(const uint8_t *, uint8_t)> esp_rc_data_callback_t;
typedef std::function<void(uint8_t mac[6], uint8_t* buf, size_t count, void* cbarg)> esp_rx_cb_t;

#define TypeVAL(t) t type

/** @brief Key length. */
static const int ESPNOW_MQ_KEYLEN = 16;

/** @brief Maximum message length. */
static const int ESPNOW_MQ_MAXMSGLEN = 200; // esp8266 max len

/** @brief Listener/Peer Status. */
enum class ListenerStatus : uint8_t {
  OFFLINE = 0, 
  ONLINE  = 1, 
  BUSY    = 2,
};

/** @brief Result of send operation. */
enum class EspNowMQSendStatus : uint8_t {
  NONE = 0, ///< result unknown, send in progress
  OK   = 1, ///< sent successfully
  FAIL = 2, ///< sending failed
};

/** @brief Listener/Peer Type. */
enum class ListenerType : uint8_t {
  UNKNOWN,
  INPUT_BINARY,
  INPUT_ANALOG,
  OUTPUT_BINARY,
  OUTPUT_ANALOG,
	EXTENDED
};

enum ListenerRole : uint8_t {
	PUB = 0,
	SUB = 1
};

enum RUN_TYPE: uint8_t {
  ONCE,
  PERIODIC
};

enum NET_ROLE : uint8_t {
	BROKER,
	LISTENER
};

enum LOGIC_TYPE : uint8_t {
  NO,
  NC,
  EQ,
  NEQ,
  LES,
  LEQ,
  GRT,
  GRQ,
  INVALID
};

enum DATA_TYPE : uint8_t {
	//Variable Exclusive Types
	TYPE_VAR_UBYTE,		//variable type, used to store information (8-bit unsigned integer)
	TYPE_VAR_USHORT,	//variable type, used to store information (16-bit unsigned integer)
	TYPE_VAR_INT,		//variable type, used to store information (integers - 32bit)
	TYPE_VAR_UINT,		//variable type, uder to store information (unsigned integers - 32bit)
	TYPE_VAR_BOOL,	    //variable type, used to store information (boolean)
	TYPE_VAR_FLOAT,		//variable type, used to store information (float/double)
	TYPE_VAR_LONG,		//variable type, used to store information (long int - 64bit)
	TYPE_VAR_ULONG,		//variable type, used to store information (unsigned long - 64bit)
	TYPE_VAR_STRING,	//variable type, used to store information (String)
};

enum WRAP_TYPE : uint8_t {
  W_SERIAL,
  W_PARALEL
};

struct VarObject {
	public:
	void *value = 0;
	DATA_TYPE dataType;
	VarObject() {};

	VarObject(void *value, DATA_TYPE type): value(value), dataType(type) {};

	VarObject(void *value): value(value), dataType(DATA_TYPE::TYPE_VAR_INT) {};

	VarObject(const VarObject& data): value(data.value), dataType(data.dataType) {};

	VarObject(std::shared_ptr<VarObject> data): VarObject(*data) {};

	const DATA_TYPE getType() {
		return dataType;
	}

	void setType(const DATA_TYPE type) {
		dataType = type;
	}

	template <class T> T getValue() {
			switch(getType())
			{
					case DATA_TYPE::TYPE_VAR_BOOL:
					case DATA_TYPE::TYPE_VAR_FLOAT:
					case DATA_TYPE::TYPE_VAR_USHORT:
					case DATA_TYPE::TYPE_VAR_INT:
					case DATA_TYPE::TYPE_VAR_UINT:
					case DATA_TYPE::TYPE_VAR_LONG:
					case DATA_TYPE::TYPE_VAR_ULONG:
					{
							return *((T *)value);
					}
					break;
					// case DATA_TYPE::TYPE_VAR_STRING:
					// {
					// 		return String(*((char *)value));
					// }
					// break;
					default:
					break;
			}

			return static_cast<T>(0);
	}

	String toString()
	{
			String lvalue;
			switch(getType()) //get the correct lvalue from the variable object
			{
					case DATA_TYPE::TYPE_VAR_USHORT:
							lvalue = getValue<uint16_t>();
					break;
					case DATA_TYPE::TYPE_VAR_UINT:
							lvalue = getValue<uint_fast32_t>();
					break;
					case DATA_TYPE::TYPE_VAR_INT:
							lvalue = getValue<int_fast32_t>();
					break;
					case DATA_TYPE::TYPE_VAR_ULONG:
							lvalue = intToStr(getValue<uint64_t>());
					break;
					case DATA_TYPE::TYPE_VAR_LONG:
							lvalue = intToStr(getValue<int64_t>());
					break;
					case DATA_TYPE::TYPE_VAR_FLOAT:
							lvalue = getValue<double>();
					break;
					case DATA_TYPE::TYPE_VAR_BOOL:
							lvalue = getValue<bool>();
					break;
					case DATA_TYPE::TYPE_VAR_STRING:
							lvalue = String(*((char *)value));
					break;
					default:
					break;
			}

			return lvalue;
	}

	template <typename T>
	void setValue( T val )
	{
			if (std::is_same<T, uint16_t>::value) {
				setType(DATA_TYPE::TYPE_VAR_USHORT);
			} else 
			if (std::is_same<T, uint_fast32_t>::value) {
				setType(DATA_TYPE::TYPE_VAR_UINT);
			} else 
			if (std::is_same<T, int_fast32_t>::value) {
				setType(DATA_TYPE::TYPE_VAR_INT);
			} else 
			if (std::is_same<T, uint64_t>::value) {
				setType(DATA_TYPE::TYPE_VAR_ULONG);
			} else 
			if (std::is_same<T, int64_t>::value) {
				setType(DATA_TYPE::TYPE_VAR_LONG);
			} else 
			if (std::is_same<T, double>::value) {
				setType(DATA_TYPE::TYPE_VAR_FLOAT);
			} else 
			if (std::is_same<T, bool>::value) {
				setType(DATA_TYPE::TYPE_VAR_BOOL);
			} else 
			if (std::is_same<T, String>::value) {
				setType(DATA_TYPE::TYPE_VAR_STRING);
			}

			switch(getType())
			{
					case DATA_TYPE::TYPE_VAR_BOOL:
					case DATA_TYPE::TYPE_VAR_FLOAT:
					case DATA_TYPE::TYPE_VAR_USHORT:
					case DATA_TYPE::TYPE_VAR_INT:
					case DATA_TYPE::TYPE_VAR_UINT:
					case DATA_TYPE::TYPE_VAR_LONG:
					case DATA_TYPE::TYPE_VAR_ULONG:
					case DATA_TYPE::TYPE_VAR_STRING:
					{
							value = &val;
					}
					break;
					default:
					break;
			}
	}

	// bool operator!() {
  //   return !value;
  // }

  bool operator<(const VarObject& data) {
    return this->getValue<double>() < VarObject(data).getValue<double>();
  }

  bool operator>(const VarObject& data) {
    return this->getValue<double>() > VarObject(data).getValue<double>();
  }

  bool operator==(const VarObject& data) {
    return (this->getValue<double>() == VarObject(data).getValue<double>());
  }

  bool operator<=(const VarObject& data) {
    return (this->getValue<double>() <= VarObject(data).getValue<double>());
  }

  bool operator>=(const VarObject& data) {
    return (this->getValue<double>() >= VarObject(data).getValue<double>());
  }

  bool operator!=(const VarObject& data) {
    return !this->operator==(data);
  }

  bool operator&&(const VarObject& data) {
    return (*this == data);
  }

  bool operator||(const VarObject& data) {
    return (value || data.value);
  }
};

struct DataObject: VarObject {
	public:
	String name;
	// void *value = 0;
	// DATA_TYPE dataType;
	DataObject(): VarObject() {};

	DataObject(String name_, void *value, DATA_TYPE type): VarObject(value, type) { name = name_;};
	DataObject(String name_, void *value): VarObject(value) { name = name_;};

	DataObject(const DataObject& data) {
		name = data.name;
		value = data.value;
		dataType = data.dataType;
	};

	DataObject(std::shared_ptr<DataObject> data): DataObject(*data) {};

};


struct ConfigObject {
	VarObject onVal;
	VarObject offVal;
	VarObject minVal;
	VarObject maxVal;
	VarObject alertMinVal;
	VarObject alertMaxVal;
	VarObject onLabel;
	VarObject offLabel;
};


struct EspNowMQPeerInfo {
  uint8_t mac[6];
  uint8_t channel;
	ListenerStatus status;
	ListenerType type;
	String listenOn; // event
	ListenerRole role;
	std::shared_ptr<DataObject> data;
	// std::map<String, VarObject> configs;
	ConfigObject config;

	//state on, of

	EspNowMQPeerInfo() {}
	EspNowMQPeerInfo(EspNowMQPeerInfo & peer) {
		memcpy(mac, peer.mac, 6);
		channel = peer.channel;
		status = peer.status;
		type = peer.type;
		listenOn = peer.listenOn;
		role = peer.role;
		data = std::make_shared<DataObject>(peer.data);
	}

	// bool hasConfig(String key) {
	// 	std::map<String, VarObject>::iterator res = configs.find(key);
	// 	if (res != configs.end()) return true;
	// 	return false;
	// }

	// template <class T> T &config(String key) {
	// 	std::map<String, VarObject>::iterator res = configs.find(key);
	// 	return res->second.getValue<T>();
	// }

	// template <class T> void config(String key, T value) {
	// 	if (std::is_same<T, uint16_t>::value) {
	// 		configs.emplace(key, VarObject(&value, DATA_TYPE::TYPE_VAR_USHORT));
	// 	} else 
	// 	if (std::is_same<T, uint_fast32_t>::value) {
	// 		configs.emplace(key, VarObject(&value, DATA_TYPE::TYPE_VAR_UINT));
	// 	} else 
	// 	if (std::is_same<T, int_fast32_t>::value) {
	// 		configs.emplace(key, VarObject(&value, DATA_TYPE::TYPE_VAR_INT));
	// 	} else 
	// 	if (std::is_same<T, uint64_t>::value) {
	// 		configs.emplace(key, VarObject(&value, DATA_TYPE::TYPE_VAR_ULONG));
	// 	} else 
	// 	if (std::is_same<T, int64_t>::value) {
	// 		configs.emplace(key, VarObject(&value, DATA_TYPE::TYPE_VAR_LONG));
	// 	} else 
	// 	if (std::is_same<T, double>::value) {
	// 		configs.emplace(key, VarObject(&value, DATA_TYPE::TYPE_VAR_FLOAT));
	// 	} else 
	// 	if (std::is_same<T, bool>::value) {
	// 		configs.emplace(key, VarObject(&value, DATA_TYPE::TYPE_VAR_BOOL));
	// 	} else 
	// 	if (std::is_same<T, String>::value) {
	// 		configs.emplace(key, VarObject(&value, DATA_TYPE::TYPE_VAR_STRING));
	// 	}
	// }

};

struct MessageObject {
	String m_event_name;
  uint8_t dst[6];
  uint8_t src[6];
	bool confirm = false;
	DataObject data;
};

struct esp_rc_event_t {
	String event_name;
	esp_rc_callback_t on_cb;
	esp_rc_data_callback_t on_data_cb;
};


#endif