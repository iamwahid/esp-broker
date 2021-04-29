#pragma once

#include "espnow_mq.h"

#include <WString.h>

class EspNowMQListenerClass
{
public:
  uint8_t new_listener[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  bool updatePeer = false;
  EspNowMQListenerClass();

  /**
   * @brief Initialize ESP-NOW with pseudo broadcast.
   * @param ssid AP SSID to announce and find peers.
   * @param channel AP channel, used if there is no STA connection.
   * @param scanFreq how often to scan for peers (milliseconds).
   * @return whether success.
   */
  bool
  begin(const char* ssid, int channel = 1, int scanFreq = 15000);

  /**
   * @brief Refresh peers if scanning is due.
   *
   * This should be invoked in Arduino sketch @c loop() function.
   */
  void
  loop();

  /** @brief Stop ESP-NOW. */
  void
  end();

  /**
   * @brief Set receive callback.
   * @param cb the callback.
   * @param arg an arbitrary argument passed to the callback.
   * @note Only one callback is allowed; this replaces any previous callback.
   */
  void
  onReceive(esp_rx_cb_t cb, void* arg);

  /**
   * @brief Broadcast a message.
   * @param buf payload.
   * @param count payload size, must not exceed @c ESPNOW_MQ_MAXMSGLEN .
   * @return whether success (message queued for transmission).
   */
  bool
  send(const uint8_t* buf, size_t count);

protected:
  void
  scan();

#if defined(ARDUINO_ARCH_ESP8266)
  static void
  processScan(void* result, int status);

  void
  processScan2(void* result, int status);
#elif defined(ARDUINO_ARCH_ESP32)
  void
  processScan();
#endif

protected:
  String m_ssid;
  int m_scanFreq;
  unsigned long m_nextScan;
  bool m_isScanning;
  uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
};

/**
 * @brief ESP-NOW pseudo broadcast.
 *
 * In pseudo broadcast mode, every node announces itself as a group member by advertising a
 * certain AP SSID. A node periodically scans other BSSIDs announcing the same SSID, and adds
 * them as ESP-NOW peers. Messages are sent to all knows peers.
 *
 * Pseudo broadcast does not depend on ESP-NOW API to support broadcast.
 */
extern EspNowMQListenerClass EspNowMQListener;

// ESPNOW_MQ_LISTENER_H
