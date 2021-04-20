#pragma once

#include "defines.h"

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
		return this->dataType;
	}

	void setType(const DATA_TYPE type) {
		this->dataType = type;
	}

	String getValue() {
		if (DATA_TYPE::TYPE_VAR_STRING) {
				return String(*((char *)this->value));
		}

		return "";
	}

	template <class T> T getValue() {
			if (std::is_same<T, long>::value && this->getType() == DATA_TYPE::TYPE_VAR_INT) {
				return *((T *)this->value);
			} else 
			if (std::is_same<T, double>::value && this->getType() == DATA_TYPE::TYPE_VAR_FLOAT) {
				return *((T *)this->value);
			} else 
			if (std::is_same<T, bool>::value) {
				if (this->getType() == DATA_TYPE::TYPE_VAR_BOOL)
					return *((bool *)this->value);
				else if (this->getType() == DATA_TYPE::TYPE_VAR_FLOAT || this->getType() == DATA_TYPE::TYPE_VAR_INT)
					return *((double *)this->value) > 0;
				else if (this->getType() == DATA_TYPE::TYPE_VAR_STRING)
					return !this->getValue().equals("");
			}

			return static_cast<T>(0);
	}

	template <typename T>
	void setValue( T val )
	{
			if (std::is_same<T, long>::value) {
				this->setType(DATA_TYPE::TYPE_VAR_INT);
			} else 
			if (std::is_same<T, double>::value) {
				this->setType(DATA_TYPE::TYPE_VAR_FLOAT);
			} else 
			if (std::is_same<T, bool>::value) {
				this->setType(DATA_TYPE::TYPE_VAR_BOOL);
			} else 
			if (std::is_same<T, String>::value) {
				this->setType(DATA_TYPE::TYPE_VAR_STRING);
			}

			this->value = &val;
	}

	String toString()
	{
			String lvalue;
			switch(this->getType()) //get the correct lvalue from the variable object
			{
					case DATA_TYPE::TYPE_VAR_INT:
							lvalue = this->getValue<long>();
					break;
					case DATA_TYPE::TYPE_VAR_FLOAT:
							lvalue = this->getValue<double>();
					break;
					case DATA_TYPE::TYPE_VAR_BOOL:
							lvalue = this->getValue<bool>();
					break;
					case DATA_TYPE::TYPE_VAR_STRING:
							lvalue = this->getValue();
					break;
					default:
					break;
			}

			return lvalue;
	}

	long toInt() {
		return this->getValue<long>();
	}

	double toFloat() {
		return this->getValue<double>();
	}

	void setBool(bool value) {
		this->setValue<bool>(value);
	}

	void setInt(long value) {
		this->setValue<long>(value);
	}

	void setFloat(double value) {
		this->setValue<double>(value);
	}

	void setString(String value) {
		this->setValue<String>(value);
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
    return (this->value || data.value);
  }

	template <class T> void operator=(const T& value) {
    setValue<T>(this->value);
  }
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

enum DATA_STATE : uint8_t {
	ON,
	OFF,
	MIN,
	MAX,
	ALERT_MIN,
	ALERT_MAX,
	UNKNOWN_STATE
};

struct DataObject: VarObject {
	public:
	String name;
	ConfigObject configs;
	DataObject(): VarObject() {
		this->dataType = DATA_TYPE::TYPE_VAR_STRING;
	};

	DataObject(String name_, void *value, DATA_TYPE type): VarObject(value, type) { this->name = name_;};
	DataObject(String name_, void *value): VarObject(value) { this->name = name_;};

	DataObject(const DataObject& data) {
		this->name = data.name;
		this->value = data.value;
		this->dataType = data.dataType;
	};

	DataObject(std::shared_ptr<DataObject> data): DataObject(*data) {};

	void setName(String name_) {
		this->name = name_;
	}

	uint8_t getState() {
		if (*this == this->configs.onVal) {
			return DATA_STATE::ON;
		} else if (*this == this->configs.offVal) {
			return DATA_STATE::OFF;
		} else if (*this == this->configs.minVal) {
			return DATA_STATE::MIN;
		} else if (*this == this->configs.maxVal) {
			return DATA_STATE::MAX;
		} else if (*this == this->configs.alertMinVal) {
			return DATA_STATE::ALERT_MIN;
		} else if (*this == this->configs.alertMaxVal) {
			return DATA_STATE::ALERT_MAX;
		}
		return DATA_STATE::UNKNOWN_STATE;
	}

	String getStateValue(uint8_t state) {
		switch (state)
		{
		case DATA_STATE::ON:
			return this->configs.onVal.toString();
			break;
		case DATA_STATE::OFF:
			return this->configs.offVal.toString();
			break;
		case DATA_STATE::MIN:
			return this->configs.minVal.toString();
			break;
		case DATA_STATE::MAX:
			return this->configs.maxVal.toString();
			break;
		case DATA_STATE::ALERT_MIN:
			return this->configs.alertMinVal.toString();
			break;
		case DATA_STATE::ALERT_MAX:
			return this->configs.alertMaxVal.toString();
			break;
		default:
			break;
		}
		return "";
	}

};

struct WifiNowPeer { // versi 1 by 1 data + config
  uint8_t mac[6];
  uint8_t channel;
	ListenerStatus status;
	ListenerType type;
	String listenOn; // event
	ListenerRole role;
	std::shared_ptr<DataObject> data;
	// std::map<String, VarObject> configs;
	ConfigObject configs;

	//state on, of

	WifiNowPeer() {}
	WifiNowPeer(WifiNowPeer & peer) {
		memcpy(this->mac, peer.mac, 6);
		this->channel = peer.channel;
		this->status = peer.status;
		this->type = peer.type;
		this->listenOn = peer.listenOn;
		this->role = peer.role;
		this->data = std::make_shared<DataObject>(peer.data);
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
	// 	if (std::is_same<T, long>::value) {
	// 		configs.emplace(key, VarObject(&value, DATA_TYPE::TYPE_VAR_INT));
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

