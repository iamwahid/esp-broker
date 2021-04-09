#include <stdio.h>
#include <WString.h>
// #include <type_traits>
// #include <typeinfo>
// #ifndef _MSC_VER
// #   include <cxxabi.h>
// #endif
// #include <memory>
// #include <string>
// #include <cstdlib>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#include "helpers.h"


std::map<uint8_t, PIN_TYPE> pinMap;
std::map<PIN_TYPE, uint8_t> usedPin;

String macStr(uint8_t * addr) {
  char macStr[18];

  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
         addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  return macStr;
}

void macHex(uint8_t * dst, char * str) {
  char* ptr;

  dst[0] = strtol(str, &ptr, 16 );
  for( uint8_t i = 1; i < 6; i++ )
  {
    dst[i] = strtol(ptr+1, &ptr, 16 );
  }
  // DEBUG
  // Serial.print(dst[0], 16);
  // for( uint8_t i = 1; i < 6; i++)
  // {
  //   Serial.print(':');
  //   Serial.print( dst[i], 16);
  // }
    // Serial.println();
  // Serial.println(str);
}

void reverse(char* begin, char* end) {
    char *is = begin;
    char *ie = end - 1;
    while(is < ie) {
        char tmp = *ie;
        *ie = *is;
        *is = tmp;
        ++is;
        --ie;
    }
}

template <typename T>
char* intToASCII(T value, char* result, uint8_t base) 
{
    if(base < 2 || base > 16) {
        *result = 0;
        return result;
    }

    char* out = result;
    T quotient = llabs(value);

    do {
        const T tmp = quotient / base;
        *out = "0123456789abcdef"[quotient - (tmp * base)];
        ++out;
        quotient = tmp;
    } while(quotient);

    // Apply negative sign
    if(value < 0)
        *out++ = '-';

    reverse(result, out);
    *out = 0;
    return result;
}

template <typename T>
String intToStr( const T value, uint8_t base)
{
	char buf[2 + 8 * sizeof(T)];
    intToASCII(value, buf, base);
    
	String str(buf);
    return str;
}

// template <class T>
// String type_name() {
//     typedef typename std::remove_reference<T>::type TR;

//     std::unique_ptr<char, void(*)(void*)> own
//            (
// #ifndef _MSC_VER
//                 abi::__cxa_demangle(typeid(TR).name(), nullptr,
//                                            nullptr, nullptr),
// #else
//                 nullptr,
// #endif
//                 std::free
//            );
//     String r = own != nullptr ? own.get() : typeid(TR).name();
//     if (std::is_const<TR>::value)
//         r += " const";
//     if (std::is_volatile<TR>::value)
//         r += " volatile";
//     if (std::is_lvalue_reference<T>::value)
//         r += "&";
//     else if (std::is_rvalue_reference<T>::value)
//         r += "&&";
//     return r;
// }

uint8_t* setUint16(uint8_t* buffer, uint16_t value, size_t offset) {
	buffer[offset + 1] = value & 0xff;
	buffer[offset + 0] = (value >> 8);
  return &buffer[offset + 2];
}

uint32_t chip_id() {
#if defined(ARDUINO_ARCH_ESP8266)
  return (uint32_t) system_get_chip_id();
#elif defined(ARDUINO_ARCH_ESP32)
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
  return chipId;
#endif
}

void setPin(uint8_t pin, PIN_TYPE Type) {
  std::map<PIN_TYPE, uint8_t>::iterator pinitr = usedPin.find(Type);
	if (pinitr != usedPin.end() && pinitr->second != 100 && pin)
	{
		pinitr->second = pin;
    // if (Type == PIN_TYPE::U_IN_PIN_1 || Type == PIN_TYPE::U_IN_PIN_2)
    //   pinMode(pin, OUTPUT);
    // else if ()
	}
}

uint8_t getPin(PIN_TYPE Type) {
  std::map<PIN_TYPE, uint8_t>::iterator pinitr = usedPin.find(Type);
	if (pinitr != usedPin.end() && pinitr->second != 100)
	{
		return pinitr->second;
	}
  return 0;
}

// uint8_t getPinAvailable(PIN_TYPE Type) {
//   std::map<PIN_TYPE, uint8_t>::iterator pinitr = usedPin.find(Type);
// 	if (pinitr == usedPin.end() || pinitr->second != 100)
// 	{
// 		for (auto it = pinMap.begin(); it != pinMap.end(); ++it)
//       if (it->second == Type)
//           return it->first;
// 	}
//   return 0;
// }

PIN_TYPE getPinType(uint8_t pin) {
  std::map<uint8_t, PIN_TYPE>::iterator pinitr = pinMap.find(pin);
	if (pinitr != pinMap.end())
	{
		return pinitr->second;
	}
  return PIN_TYPE::INVALID;
}