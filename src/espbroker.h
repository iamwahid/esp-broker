#ifndef _ESPBROKER_H_
#define _ESPBROKER_H_

#include "espbroker/config.h"

#if defined(ESPBROKER_AS_BROKER)
#include "espbroker/broker.h"
#include "espbroker/event.h"
#elif defined(ESPBROKER_AS_LISTENER)
#include "espbroker/listener.h"
#else
#error "Define role as BROKER or LISTENER"
#endif


#endif