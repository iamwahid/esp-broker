#ifndef __WIFI_MESH_RC_H__
#define __WIFI_MESH_RC_H__

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#include <cstddef>
#include <cstdint>

typedef std::function<void(void)> esp_rc_callback_t;
typedef std::function<void(u8 *, u8)> esp_rc_data_callback_t;

struct esp_rc_event_t {
	String prefix;
	esp_rc_callback_t callback;
	esp_rc_data_callback_t callback2;
};

class MeshRCClass {
	public:
		MeshRCClass();

		void wait();

		void delayMs(u32 time);

		bool equals(u8 *a, u8 *b, u8 size, u8 offset);

		void begin();

		using RxCallback = void (*)(const uint8_t mac[6], const uint8_t* buf, size_t count, void* cbarg);

		void setMaster(u8 *addr);

		void send(u8 *data, u8 size);

		void send(String data);

		void send(String type, u8 *data, u8 size);

		void on(String prefix, esp_rc_callback_t callback);

		void on(String prefix, esp_rc_data_callback_t callback);
		

	private:
		static void recvHandler(u8 *addr, u8 *data, u8 size);
		
		static void sendHandler(u8 *addr, u8 err);
		// esp_now_send_cb_t sendHandler;
		// esp_now_recv_cb_t recvHandler;
		
	private:
		u8 buffer[250];
		u8 broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
		u8 events_num = 0;
		u32 received;
		u32 ignored;
		u8 *sender = NULL;
		u32 sendTime;
		u16 duration;
		u8 *master = NULL;
		bool sending;
};

/** @brief ESP-NOW API. */
extern MeshRCClass EspNowMQ;

#endif	//__WIFI_MESH_RC_H__
