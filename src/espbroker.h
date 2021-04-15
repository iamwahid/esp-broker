#pragma once

#include "espbroker/config.h"

#if defined(ESPBROKER_AS_BROKER)
#include "espbroker/broker.h"
#include "espbroker/events/blink.h"
#include "espbroker/events/switch.h"
#elif defined(ESPBROKER_AS_LISTENER)
#include "espbroker/listeners/acmeter/acmeter.h"
#include "espbroker/listeners/blink/blink.h"
#if defined(ARDUINO_ARCH_ESP32)
#include "espbroker/listeners/camera/espcam.h"
// #include "espbroker/listeners/camera/espcam_pir.h"
#endif
#include "espbroker/listeners/switch/switch.h"
#else
#error "Define role as BROKER or LISTENER"
#endif


