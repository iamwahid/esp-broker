#ifndef _LISTENER_H_
#define _LISTENER_H_

#include "./listeners/acmeter/acmeter.h"
#include "./listeners/blink/blink.h"
#if defined(ARDUINO_ARCH_ESP32)
#include "./listeners/camera/espcam.h"
// #include "./listeners/camera/espcam_pir.h"
#endif
#include "./listeners/switch/switch.h"

#endif