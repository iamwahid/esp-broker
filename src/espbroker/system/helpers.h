#ifndef _HELPERS_H_
#define _HELPERS_H_

#include "defines.h"

#define SET_PIN(MODULE_TYPE) \
  do { \
    if (MODULE_TYPE == ListenerType::OUTPUT_ANALOG) { \
      OUTPUT_PIN_1 = PWM_PIN_1; \
      OUTPUT_PIN_2 = PWM_PIN_2; \
      INPUT_PIN_1 = PWM_PIN_3; \
      INPUT_PIN_2 = PWM_PIN_4; \
      pinMode(OUTPUT_PIN_1, OUTPUT); \
      pinMode(OUTPUT_PIN_2, OUTPUT); \
      pinMode(INPUT_PIN_1, INPUT); \
      pinMode(INPUT_PIN_2, INPUT); \
    } else if (MODULE_TYPE == ListenerType::OUTPUT_BINARY) { \
      OUTPUT_PIN_1 = PWM_PIN_1; \
      OUTPUT_PIN_2 = PWM_PIN_2; \
      INPUT_PIN_1 = PWM_PIN_3; \
      INPUT_PIN_2 = PWM_PIN_4; \
      pinMode(OUTPUT_PIN_1, OUTPUT); \
      pinMode(OUTPUT_PIN_2, OUTPUT); \
      pinMode(INPUT_PIN_1, INPUT_PULLUP); \
      pinMode(INPUT_PIN_2, INPUT_PULLUP); \
    } else if (MODULE_TYPE == ListenerType::INPUT_ANALOG) { \
      INPUT_PIN_1 = ANALOG_PIN; \
      pinMode(INPUT_PIN_1, INPUT); \
    } else if (MODULE_TYPE == ListenerType::INPUT_BINARY) { \
      INPUT_PIN_1 = PWM_PIN_1; \
      pinMode(INPUT_PIN_1, INPUT); \
    } else if (MODULE_TYPE == ListenerType::EXTENDED) { \
      INPUT_PIN_1 = PWM_PIN_1; \
      INPUT_PIN_2 = PWM_PIN_2; \
      OUTPUT_PIN_1 = PWM_PIN_3; \
      OUTPUT_PIN_2 = PWM_PIN_4; \
      pinMode(OUTPUT_PIN_1, OUTPUT); \
      pinMode(OUTPUT_PIN_2, OUTPUT); \
      pinMode(INPUT_PIN_1, INPUT); \
      pinMode(INPUT_PIN_2, INPUT); \
    } \
  } while (false)

String macStr(uint8_t * addr);

void macHex(uint8_t * dst, char * str);

void reverse(char* begin, char* end);

template <typename T>
char* intToASCII(T value, char* result, uint8_t base = 10);

template <typename T>
String intToStr( const T value, uint8_t base = 10 );

template <class T> String type_name();

uint8_t* setUint16(uint8_t* buffer, uint16_t value, size_t offset = 0);

uint32_t chip_id();


void setPin(uint8_t pin, PIN_TYPE Type);

uint8_t getPin(PIN_TYPE Type);

// uint8_t getPinAvailable(PIN_TYPE Type);

PIN_TYPE getPinType(uint8_t pin);

// #if defined(ARDUINO_ARCH_ESP8266)
// // esp8266
// void resetPinMap() {
//   pinMap.clear();
//   pinMap.emplace(16, PIN_TYPE::WAKEUP_PIN);
//   // pinMap.emplace(D4, PIN_TYPE::PWM_PIN);
//   // pinMap.emplace(D5, PIN_TYPE::PWM_PIN);
//   // pinMap.emplace(D6, PIN_TYPE::PWM_PIN);
//   // pinMap.emplace(D7, PIN_TYPE::PWM_PIN);
//   // pinMap.emplace(D8, PIN_TYPE::PWM_PIN);
//   pinMap.emplace(A0, PIN_TYPE::ANALOG_PIN);

//   usedPin.clear();
//   usedPin.emplace(PIN_TYPE::U_IN_PIN_1, 100);
//   usedPin.emplace(PIN_TYPE::U_IN_PIN_2, 100);
//   usedPin.emplace(PIN_TYPE::U_OUT_PIN_1, 100);
//   usedPin.emplace(PIN_TYPE::U_OUT_PIN_2, 100);
// }
// const int WAKEUP_PIN = 16; // flash/boot button
// const int PWM_PIN_1 = 2; // esp01 1 led builtin
// const int PWM_PIN_2 = 14;
// const int PWM_PIN_3 = 12;
// const int PWM_PIN_4 = 13;
// // const int PWM_PIN_5 = 15;
// const int ANALOG_PIN = A0;
// #elif defined(ARDUINO_ARCH_ESP32)
// // esp32
// void resetPinMap() {
//   pinMap.clear();
//   pinMap.emplace(16, PIN_TYPE::WAKEUP_PIN);
//   pinMap.emplace(13, PIN_TYPE::PWM_PIN);
//   pinMap.emplace(14, PIN_TYPE::PWM_PIN);
//   pinMap.emplace(27, PIN_TYPE::PWM_PIN);
//   pinMap.emplace(26, PIN_TYPE::PWM_PIN);
//   pinMap.emplace(33, PIN_TYPE::PWM_PIN);
//   pinMap.emplace(25, PIN_TYPE::ANALOG_PIN);

//   usedPin.clear();
//   usedPin.emplace(PIN_TYPE::U_IN_PIN_1, 100);
//   usedPin.emplace(PIN_TYPE::U_IN_PIN_2, 100);
//   usedPin.emplace(PIN_TYPE::U_OUT_PIN_1, 100);
//   usedPin.emplace(PIN_TYPE::U_OUT_PIN_2, 100);
// }

#endif