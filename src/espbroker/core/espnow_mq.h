#pragma once

// #define MQ_DEBUG_ALL_MSG 1
#define MQ_DEBUG_CONFIRMATION 1
// #define MQ_DEBUG 1

#if defined(MQ_DEBUG)
 #define DEBUG_PRINT(x) \
 do { \
 	Serial.print("DEBUG:: "); \
	Serial.print(x); \
 } while (false)
#else
 #define DEBUG_PRINT(x)
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#include "defines.h"
#include "objects.h"

class EventObject;

class EspNowMQClass {
	public:
		const static int MAX_PEERS = 20;
		WifiNowPeer peerListeners[MAX_PEERS];
		int listenerCount = 0;
		uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

		EspNowMQClass();

		void wait();

		void delayMs(uint32_t time);
		
		bool equals(const uint8_t *a, const uint8_t *b, uint8_t size, uint8_t offset = 0);
		
		bool begin(uint8_t role = 0, const char * SSID = "ESPNOW_MQ_NET");

		void end();

		/**
		 * @brief List current peers.
		 * @param[out] peers buffer for peer information.
		 * @param maxPeers buffer size.
		 * @return total number of peers, @c std::min(retval,maxPeers) is written to @p peers .
		 */
		int listPeers(WifiNowPeer* peers, int maxPeers) const;

		bool refreshPeers();

		/**
		 * @brief Test whether peer exists.
		 * @param mac peer MAC address.
		 * @return whether peer exists.
		 */
		bool hasPeer(const uint8_t mac[6]) const;

		int indexOf(const uint8_t mac[6]);

		void updatePeer(WifiNowPeer * peers, const uint8_t mac[6], String listenOn);
		
		void updatePeer(WifiNowPeer * peers, const uint8_t mac[6], ListenerType type);

		/**
		 * @brief Add a peer or change peer channel.
		 * @param mac peer MAC address.
		 * @param channel peer channel, 0 for current channel.
		 * @param key encryption key, nullptr to disable encryption.
		 * @param netif (ESP32 only) WiFi interface.
		 * @return whether success.
		 * @note To change peer key, remove the peer and re-add.
		 */
#if defined(ARDUINO_ARCH_ESP8266)
		bool addPeer(const uint8_t mac[6], int channel = 0, const uint8_t key[ESPNOW_MQ_KEYLEN] = nullptr);
#elif defined(ARDUINO_ARCH_ESP32)
		bool addPeer(const uint8_t mac[6], int channel = 0, const uint8_t key[ESPNOW_MQ_KEYLEN] = nullptr, int netif = ESP_IF_WIFI_AP); //WIFI_IF_STA | ESP_IF_WIFI_AP | ESP_IF_WIFI_STA
#endif

		/**
		 * @brief Remove a peer.
		 * @param mac peer MAC address.
		 * @return whether success.
		 */
		bool removePeer(const uint8_t mac[6]);

		// using esp_rx_cb_t = void (*)(uint8_t mac[6], uint8_t* buf, size_t count, void* cbarg);

		/**
		 * @brief Set receive callback.
		 * @param cb the callback.
		 * @param arg an arbitrary argument passed to the callback.
		 * @note Only one callback is allowed; this replaces any previous callback.
		 */
		void onReceive(esp_rx_cb_t cb, void* arg);

		void setMaster(uint8_t *addr);

		/**
		 * @brief Send a message.
		 * @param mac destination MAC address, nullptr for all peers.
		 * @param buf payload.
		 * @param count payload size, must not exceed @p ESPNOW_MQ_MAXMSGLEN .
		 * @return whether success (message queued for transmission).
		 */
		bool send(const uint8_t mac[6], const uint8_t* buf, size_t count);

		bool send(const String type, const uint8_t dst[6], const String name, void *value);

		void on(String event_name, esp_rc_data_callback_t data_callback, esp_rc_callback_t callback = NULL);

		void on(String event_name, std::shared_ptr<EventObject> event_obj);
		void on(String event_name);

		EventObject &getEvent(String event_name);

		EspNowMQSendStatus getSendStatus() const
		{
			return m_txRes;
		}
		

	protected:
		static void recvHandler(const uint8_t *addr, const uint8_t *data, uint8_t size);
		static void sendHandler(const uint8_t *addr, uint8_t err);
		
	protected:
		uint8_t buffer[250];
		uint8_t events_num = 0;
		uint32_t received;
		uint32_t ignored;
		uint32_t Dchannel;
		uint8_t *sender = NULL;
		uint32_t sendTime;
		uint16_t duration_s;
		uint16_t duration_c;
		uint8_t *master = NULL;
		uint8_t *mac = NULL;
		bool sending;

		esp_rx_cb_t m_rxCb = nullptr;
		void* m_rxArg = nullptr;
		EspNowMQSendStatus m_txRes;
		bool m_ready = false;
};

/** @brief ESP-NOW API. */
extern EspNowMQClass EspNowMQ;

//ESPNOW_MQ_H
