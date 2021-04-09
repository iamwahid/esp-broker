#include "EspNowMQBroadcast.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <user_interface.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

EspNowMQBroadcastClass EspNowMQBroadcast;

EspNowMQBroadcastClass::EspNowMQBroadcastClass()
  : m_isScanning(false)
{}

bool EspNowMQBroadcastClass::begin(const char* ssid, int channel, int scanFreq)
{
  m_ssid = ssid;
  m_nextScan = 0;
  m_scanFreq = scanFreq;

  // AP mode for announcing our presence, STA mode for scanning
  WiFi.mode(WIFI_AP_STA);
  // disconnect from any previously saved SSID, so that the specified channel can take effect
  WiFi.disconnect();
  // establish AP at the specified channel to announce our presence
  WiFi.softAP(ssid, nullptr, channel);

  return EspNowMQ.begin();
}

void EspNowMQBroadcastClass::loop()
{
  if (millis() >= m_nextScan && !m_isScanning && WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
    this->scan();
  }
#ifdef ARDUINO_ARCH_ESP32
  if (m_isScanning && WiFi.scanComplete() >= 0) {
    this->processScan();
  }
#endif
}

void EspNowMQBroadcastClass::end()
{
  EspNowMQ.end();
  WiFi.softAPdisconnect();
  m_ssid = "";
}

void EspNowMQBroadcastClass::onReceive(esp_rx_cb_t cb, void* arg)
{
  EspNowMQ.onReceive(cb, arg);
}

bool EspNowMQBroadcastClass::send(const uint8_t* buf, size_t count)
{
  return EspNowMQ.send(nullptr, buf, count);
}

void EspNowMQBroadcastClass::scan()
{
  m_isScanning = true;
#if defined(ARDUINO_ARCH_ESP8266)
  scan_config sc;
#elif defined(ARDUINO_ARCH_ESP32)
  wifi_scan_config_t sc;
#endif
  memset(&sc, 0, sizeof(sc));
  sc.ssid = reinterpret_cast<uint8_t*>(const_cast<char*>(m_ssid.c_str()));
#if defined(ARDUINO_ARCH_ESP8266)
  wifi_station_scan(&sc, reinterpret_cast<scan_done_cb_t>(EspNowMQBroadcastClass::processScan));
#elif defined(ARDUINO_ARCH_ESP32)
  esp_wifi_scan_start(&sc, false);
#endif
}

#if defined(ARDUINO_ARCH_ESP8266)
void EspNowMQBroadcastClass::processScan(void* result, int status)
{
  EspNowMQBroadcast.processScan2(result, status);
}

void EspNowMQBroadcastClass::processScan2(void* result, int status)

#define FOREACH_AP(f)                                                                              \
  do {                                                                                             \
    for (bss_info* it = reinterpret_cast<bss_info*>(result); it; it = STAILQ_NEXT(it, next)) {     \
      (f)(it->bssid, it->channel);                                                                 \
    }                                                                                              \
  } while (false)

#define DELETE_APS                                                                                 \
  do {                                                                                             \
  } while (false)

#elif defined(ARDUINO_ARCH_ESP32)
void EspNowMQBroadcastClass::processScan()

// ESP32 WiFiScanClass::_scanDone is always invoked after a scan complete event, so we can use
// Arduino's copy of AP records, but we must check SSID, and should not always delete AP records.

#define FOREACH_AP(f)                                                                              \
  do {                                                                                             \
    int nNetworks = WiFi.scanComplete();                                                           \
    for (uint8_t i = 0; static_cast<int>(i) < nNetworks; ++i) {                                    \
      if (WiFi.SSID(i) != m_ssid) {                                                                \
        continue;                                                                                  \
      }                                                                                            \
      (f)(WiFi.BSSID(i), static_cast<uint8_t>(WiFi.channel(i)));                                   \
    }                                                                                              \
  } while (false)

#define DELETE_APS                                                                                 \
  do {                                                                                             \
    bool hasOtherSsid = false;                                                                     \
    int nNetworks = WiFi.scanComplete();                                                           \
    for (uint8_t i = 0; static_cast<int>(i) < nNetworks; ++i) {                                    \
      if (WiFi.SSID(i) == m_ssid) {                                                                \
        continue;                                                                                  \
      }                                                                                            \
      hasOtherSsid = true;                                                                         \
      break;                                                                                       \
    }                                                                                              \
    if (!hasOtherSsid) {                                                                           \
      WiFi.scanDelete();                                                                           \
    }                                                                                              \
  } while (false)

#endif
{
  m_isScanning = false;
  m_nextScan = millis() + m_scanFreq;
#ifdef ARDUINO_ARCH_ESP8266
  if (status != 0) {
    return;
  }
#endif

  const int MAX_PEERS = 20;
  EspNowMQPeerInfo oldPeers[MAX_PEERS];
  int nOldPeers = std::min(EspNowMQ.listPeers(oldPeers, MAX_PEERS), MAX_PEERS);
  const uint8_t PEER_FOUND = 0xFF; // assigned to .channel to indicate peer is matched

  FOREACH_AP([&](const uint8_t* bssid, uint8_t channel) {
    for (int i = 0; i < nOldPeers; ++i) {
      if (memcmp(bssid, oldPeers[i].mac, 6) != 0) {
        continue;
      }
      oldPeers[i].channel = PEER_FOUND;
      break;
    }
  });

  for (int i = 0; i < nOldPeers; ++i) {
    if (oldPeers[i].channel != PEER_FOUND) {
      EspNowMQ.removePeer(oldPeers[i].mac);
    }
  }

#if defined(ARDUINO_ARCH_ESP8266)
  FOREACH_AP([&](const uint8_t* bssid, uint8_t channel) { EspNowMQ.addPeer(bssid, channel); });
#elif defined(ARDUINO_ARCH_ESP32)
  FOREACH_AP([&](const uint8_t* bssid, uint8_t channel) { EspNowMQ.addPeer(bssid, channel, nullptr, ESP_IF_WIFI_AP); });
#endif
  DELETE_APS;
}
