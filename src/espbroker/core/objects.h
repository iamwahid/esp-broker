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
		return dataType;
	}

	void setType(const DATA_TYPE type) {
		dataType = type;
	}

	String getValue() {
		if (DATA_TYPE::TYPE_VAR_STRING) {
				return String(*((char *)value));
		}

		return "";
	}

	template <class T> T getValue() {
			if (std::is_same<T, long>::value && getType() == DATA_TYPE::TYPE_VAR_INT) {
				return *((T *)value);
			} else 
			if (std::is_same<T, double>::value && getType() == DATA_TYPE::TYPE_VAR_FLOAT) {
				return *((T *)value);
			} else 
			if (std::is_same<T, bool>::value) {
				if (getType() == DATA_TYPE::TYPE_VAR_BOOL)
					return *((bool *)value);
				else if (getType() == DATA_TYPE::TYPE_VAR_FLOAT || getType() == DATA_TYPE::TYPE_VAR_INT)
					return *((double *)value) > 0;
				else if (getType() == DATA_TYPE::TYPE_VAR_STRING)
					return !getValue().equals("");
			}

			return static_cast<T>(0);
	}

	template <typename T>
	void setValue( T val )
	{
			if (std::is_same<T, long>::value) {
				setType(DATA_TYPE::TYPE_VAR_INT);
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

			value = &val;
	}

	String toString()
	{
			String lvalue;
			switch(getType()) //get the correct lvalue from the variable object
			{
					case DATA_TYPE::TYPE_VAR_INT:
							lvalue = getValue<long>();
					break;
					case DATA_TYPE::TYPE_VAR_FLOAT:
							lvalue = getValue<double>();
					break;
					case DATA_TYPE::TYPE_VAR_BOOL:
							lvalue = getValue<bool>();
					break;
					case DATA_TYPE::TYPE_VAR_STRING:
							lvalue = getValue();
					break;
					default:
					break;
			}

			return lvalue;
	}

	long toInt() {
		return getValue<long>();
	}

	double toFloat() {
		return getValue<double>();
	}

	void setBool(bool value) {
		setValue<bool>(value);
	}

	void setInt(long value) {
		setValue<long>(value);
	}

	void setFloat(double value) {
		setValue<double>(value);
	}

	void setString(String value) {
		setValue<String>(value);
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

	template <class T> void operator=(const T& value) {
    setValue<T>(value);
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

struct DataObject: VarObject {
	public:
	String name;
	ConfigObject configs;
	DataObject(): VarObject() {};

	DataObject(String name_, void *value, DATA_TYPE type): VarObject(value, type) { this->name = name_;};
	DataObject(String name_, void *value): VarObject(value) { this->name = name_;};

	DataObject(const DataObject& data) {
		name = data.name;
		value = data.value;
		dataType = data.dataType;
	};

	DataObject(std::shared_ptr<DataObject> data): DataObject(*data) {};

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

