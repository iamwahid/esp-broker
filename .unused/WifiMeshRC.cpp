#include "EspNowMQ.h"
#include <string.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <c_types.h>
#include <espnow.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <esp_now.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

 esp_rc_event_t rc_events[250];

MeshRCClass EspNowMQ;

MeshRCClass::MeshRCClass() = default;

void MeshRCClass::setMaster(u8 *addr) {
	if (esp_now_is_peer_exist(master))
		esp_now_del_peer(master);
	master = addr;
	esp_now_add_peer(master, ESP_NOW_ROLE_COMBO, 1, 0, 0);
}

void MeshRCClass::send(u8 *data, u8 size) {
	sending = true;
	sendTime = micros();
	esp_now_send(master ? master : broadcast, data, size);
}

void MeshRCClass::send(String data) {
	send((u8 *)data.c_str(), data.length());
}

void MeshRCClass::send(String type, u8 *data, u8 size) {
	memcpy(&buffer[0], (u8 *)type.c_str(), type.length());
	memcpy(&buffer[type.length()], data, size);
	send(buffer, type.length() + size);
}

void MeshRCClass::on(String prefix, esp_rc_callback_t callback) {
	rc_events[events_num++] = (esp_rc_event_t){prefix, callback, NULL};
}

void MeshRCClass::on(String prefix, esp_rc_data_callback_t callback) {
	rc_events[events_num++] = (esp_rc_event_t){prefix, NULL, callback};
}

void MeshRCClass::wait() {
	while (sending) yield();  // Wait while sending
}

void MeshRCClass::delayMs(u32 time) {
	u32 delayUntil = millis() + time;
	while (millis() < delayUntil) yield();
}

bool MeshRCClass::equals(u8 *a, u8 *b, u8 size, u8 offset = 0) {
	for (auto i = offset; i < offset + size; i++)
		if (a[i] != b[i])
			return false;
	return true;
}

void MeshRCClass::sendHandler(u8 *addr, u8 err) {
	EspNowMQ.sending = false;
	EspNowMQ.duration = micros() - EspNowMQ.sendTime;
};

void MeshRCClass::recvHandler(u8 *addr, u8 *data, u8 size) {
	static u8 offset, i;
#ifdef MESH_RC_DEBUG_ALL_MSG
	Serial.printf(":: MESH_RC RECV %u bytes: ", size);
	for (i = 0; i < size; i++) {
		Serial.write(data[i]);
	}
	Serial.printf(" [ ");
	for (i = 0; i < size; i++) {
		Serial.printf("%02X ", data[i]);
	}
	Serial.println("]");
#endif
	// Only receives from master if set
	if (addr == NULL || EspNowMQ.master == NULL || EspNowMQ.equals(addr, EspNowMQ.master, 6)) {
		EspNowMQ.received++;
		EspNowMQ.sender = addr;
		for (i = 0; i < EspNowMQ.events_num; i++) {
			offset = rc_events[i].prefix.length();
			if (EspNowMQ.equals(data, (u8 *)rc_events[i].prefix.c_str(), offset)) {
				if (rc_events[i].callback) rc_events[i].callback();
				if (rc_events[i].callback2) rc_events[i].callback2(&data[offset], size - offset);
			}
		}
	} else {
		EspNowMQ.ignored++;
	}
};

void MeshRCClass::begin() {
	if (esp_now_init() == OK) {
		if (esp_now_is_peer_exist(broadcast))
			esp_now_del_peer(broadcast);
		esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
		esp_now_add_peer(broadcast, ESP_NOW_ROLE_COMBO, 1, 0, 0);
		esp_now_register_send_cb(reinterpret_cast<esp_now_send_cb_t>(MeshRCClass::sendHandler));
		esp_now_register_recv_cb(reinterpret_cast<esp_now_recv_cb_t>(MeshRCClass::recvHandler));
	}
}
