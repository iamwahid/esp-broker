#ifndef _DEFINES_H_
#define _DEFINES_H_
#include <type_traits>

#include <map>
#include <memory>
#include <WString.h>
#include <cstddef>
#include <cstdint>
#include <pins_arduino.h>

#define TypeVAL(t) t type

typedef std::function<void(void)> esp_rc_callback_t;
typedef std::function<void(const uint8_t *, uint8_t)> esp_rc_data_callback_t;
typedef std::function<void(uint8_t mac[6], uint8_t* buf, size_t count, void* cbarg)> esp_rx_cb_t;


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
	// TYPE_VAR_UBYTE,		//variable type, used to store information (8-bit unsigned integer)
	// TYPE_VAR_USHORT,	//variable type, used to store information (16-bit unsigned integer)
	TYPE_VAR_INT,		//variable type, used to store information (integers - 32bit)
	// TYPE_VAR_UINT,		//variable type, uder to store information (unsigned integers - 32bit)
	TYPE_VAR_BOOL,	    //variable type, used to store information (boolean)
	TYPE_VAR_FLOAT,		//variable type, used to store information (float/double)
	// TYPE_VAR_LONG,		//variable type, used to store information (long int - 64bit)
	// TYPE_VAR_ULONG,		//variable type, used to store information (unsigned long - 64bit)
	TYPE_VAR_STRING,	//variable type, used to store information (String)
};

enum WRAP_TYPE : uint8_t {
  W_SERIAL,
  W_PARALEL
};

enum class PIN_TYPE: uint8_t {
  WAKEUP_PIN,
  PWM_PIN,
  ANALOG_PIN,
  U_IN_PIN_1,
  U_IN_PIN_2,
  U_OUT_PIN_1,
  U_OUT_PIN_2,
  TAKEN,
  INVALID,
};

///
extern std::map<uint8_t, PIN_TYPE> pinMap;
extern std::map<PIN_TYPE, uint8_t> usedPin;

#define DEFAULT_BUTTON 0 // flash/boot button
#define WAKEUP_PIN 16 // flash/boot button
#define PWM_PIN_1 12
#define PWM_PIN_2 13
#define PWM_PIN_3 14
#define PWM_PIN_4 15
// #define PWM_PIN_5 4
#define ANALOG_PIN 4

extern unsigned int INPUT_PIN_1; 
extern unsigned int INPUT_PIN_2; 
extern unsigned int OUTPUT_PIN_1;
extern unsigned int OUTPUT_PIN_2;
extern unsigned int USED_PIN;

#endif