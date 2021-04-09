#include "EspNowMQ.h"
#include <string.h>
#include <map>
#include "objects.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <c_types.h>
#include <espnow.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <esp_now.h>
#include <esp_wifi.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

esp_rc_event_t rc_events[250];

EspNowMQClass EspNowMQ;

MessageObject txMsg;


// ESPNOWMQ
EspNowMQClass::EspNowMQClass() = default;

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

void EspNowMQClass::setMaster(uint8_t *addr) {
	if (esp_now_is_peer_exist(master))
		esp_now_del_peer(master);
	master = addr;

  EspNowMQ.addPeer(master);
}

bool EspNowMQClass::send(const uint8_t mac[6], const uint8_t* buf, size_t count)
{
  if (!m_ready || count > ESPNOW_MQ_MAXMSGLEN || count == 0) {
    return false;
  }

  sending = true;
  sendTime = micros();

  DEBUG_PRINT("sending...\n");
  EspNowMQ.m_txRes = EspNowMQSendStatus::NONE;
  return esp_now_send(const_cast<uint8_t*>(mac), const_cast<uint8_t*>(buf),
                      static_cast<int>(count)) == 0;
}

bool EspNowMQClass::send(const String type, const uint8_t dst[6], const String name, void *value) {
  if (m_ready) {
    txMsg.m_event_name = type;
    memcpy(txMsg.dst, dst, 6);
#if defined(ARDUINO_ARCH_ESP8266)
    wifi_get_macaddr(SOFTAP_IF, txMsg.src); // mode
#elif defined(ARDUINO_ARCH_ESP32)
    esp_wifi_get_mac(WIFI_IF_AP, txMsg.src);
#endif

    txMsg.data.name = name;
    txMsg.data.setInt(*((double *) value));

    if (sizeof(txMsg) > ESPNOW_MQ_MAXMSGLEN || sizeof(txMsg) == 0) {
      DEBUG_PRINT("Size Exceeded\n");
      return false;
    }

    sending = true;
    sendTime = micros();

    return esp_now_send(const_cast<uint8_t*>(dst), (uint8_t *) &txMsg, sizeof(txMsg)) == 0;
  }
  DEBUG_PRINT("Send Failed\n");
  return false;
}

void EspNowMQClass::wait() {
	while (sending) yield();  // Wait while sending
}

void EspNowMQClass::delayMs(uint32_t time) {
	uint32_t delayUntil = millis() + time;
	while (millis() < delayUntil) yield();
}

bool EspNowMQClass::equals(const uint8_t *a, const uint8_t *b, uint8_t size, uint8_t offset) {
	for (auto i = offset; i < offset + size; i++)
		if (a[i] != b[i])
			return false;
	return true;
}

int EspNowMQClass::listPeers(EspNowMQPeerInfo* peers, int maxPeers) const
{
	if (!m_ready) {
    return 0;
  }
  int n = 0;
#if defined(ARDUINO_ARCH_ESP8266)
  for (u8* mac = esp_now_fetch_peer(true); mac != nullptr; mac = esp_now_fetch_peer(false)) {
    uint8_t channel = static_cast<uint8_t>(esp_now_get_peer_channel(mac));
#elif defined(ARDUINO_ARCH_ESP32)
  esp_now_peer_info_t peer;
  for (esp_err_t e = esp_now_fetch_peer(true, &peer); e == ESP_OK;
       e = esp_now_fetch_peer(false, &peer)) {
    uint8_t* mac = peer.peer_addr;
    uint8_t channel = peer.channel;
#endif
    if (n < maxPeers) {
      memcpy(peers[n].mac, mac, 6);
      peers[n].channel = channel;
      // peers[n].status = ListenerStatus::ONLINE;
      // peers[n].type = ListenerType::INPUT_BINARY;
      // peers[n].listenOn = "DEFAULT";
    }
    ++n;
  }
  return n;
}

bool EspNowMQClass::refreshPeers() {
  if (m_ready) {
    EspNowMQ.listenerCount = std::min(EspNowMQ.listPeers(EspNowMQ.peerListeners, EspNowMQ.MAX_PEERS), EspNowMQ.MAX_PEERS);
    return EspNowMQ.listenerCount > 0;
  }
  return false;
}

void EspNowMQClass::updatePeer(EspNowMQPeerInfo * peers, const uint8_t mac[6], String listenOn) const
{
  if (m_ready && this->hasPeer(mac)) {
    for (int i = 0; i < listenerCount; i++)
    {
       if (EspNowMQ.equals(peers[i].mac, mac, 6)) {
         peers[i].listenOn = listenOn;
         DEBUG_PRINT(listenOn+"\n");
         break;
       }
    }
  }
}
		
void EspNowMQClass::updatePeer(EspNowMQPeerInfo * peers, const uint8_t mac[6], ListenerType type) const
{
  if (m_ready && this->hasPeer(mac)) {
    for (int i = 0; i < listenerCount; i++)
    {
       if (EspNowMQ.equals(peers[i].mac, mac, 6)) {
         peers[i].type = type;
         break;
       }
    }
  }
}

#if defined(ARDUINO_ARCH_ESP8266)
bool EspNowMQClass::addPeer(const uint8_t mac[6], int channel, const uint8_t key[ESPNOW_MQ_KEYLEN])
{
  if (!m_ready) {
    return false;
  }

  if (this->hasPeer(mac)) {
    if (esp_now_get_peer_channel(const_cast<u8*>(mac)) == channel) {
      return true;
    }
    this->removePeer(mac);
  }
  return esp_now_add_peer(const_cast<u8*>(mac), ESP_NOW_ROLE_COMBO, static_cast<u8>(Dchannel ? Dchannel : channel),
                          const_cast<u8*>(key), key == nullptr ? 0 : ESPNOW_MQ_KEYLEN) == 0;
}
#elif defined(ARDUINO_ARCH_ESP32)
bool EspNowMQClass::addPeer(const uint8_t mac[6], int channel, const uint8_t key[ESPNOW_MQ_KEYLEN],
                         int netif)
{
  if (!m_ready) {
    return false;
  }

  esp_now_peer_info_t pi;
  if (esp_now_get_peer(mac, &pi) == ESP_OK) {
    if (pi.channel == static_cast<uint8_t>(channel)) {
      return true;
    }
    this->removePeer(mac);
  }
  memset(&pi, 0, sizeof(pi));
  memcpy(pi.peer_addr, mac, ESP_NOW_ETH_ALEN);
  if (Dchannel) 
    pi.channel = Dchannel;
  else
    pi.channel = static_cast<uint8_t>(channel);

  pi.ifidx = static_cast<wifi_interface_t>(netif);
  if (key != nullptr) {
    memcpy(pi.lmk, key, ESP_NOW_KEY_LEN);
    pi.encrypt = true;
  } 
  // else pi.encrypt = false;
  return esp_now_add_peer(&pi) == ESP_OK;
}
#endif

bool EspNowMQClass::hasPeer(const uint8_t mac[6]) const
{
  return m_ready && esp_now_is_peer_exist(const_cast<uint8_t*>(mac));
}

int EspNowMQClass::indexOf(const uint8_t mac[6])
{
  if (this->hasPeer(mac)) {
    for (int i = 0; i < listenerCount; i++)
    {
       if (EspNowMQ.equals(peerListeners[i].mac, mac, 6)) {
        return i;
       }
    }
  }
  return -1;
}

bool EspNowMQClass::removePeer(const uint8_t mac[6])
{
  return m_ready && esp_now_del_peer(const_cast<uint8_t*>(mac)) == 0;
}

void EspNowMQClass::onReceive(esp_rx_cb_t cb, void* arg)
{
  m_rxCb = cb;
  m_rxArg = arg;
}

void EspNowMQClass::end()
{
  if (!m_ready) {
    return;
  }
  esp_now_deinit();
  m_ready = false;
}

bool EspNowMQClass::begin(uint8_t role, const char * SSID) {
	end();

  Dchannel = getWiFiChannel(SSID);

  if (role == NET_ROLE::BROKER) {
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("ESPNOW", nullptr, 3);
    WiFi.softAPdisconnect(false);
  } else if (role == NET_ROLE::LISTENER) {
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // WiFi.printDiag(Serial); // Uncomment to verify channel number before
#if defined(ARDUINO_ARCH_ESP8266)
    wifi_promiscuous_enable(1);
    wifi_set_channel(Dchannel);
    wifi_promiscuous_enable(0);
#elif defined(ARDUINO_ARCH_ESP32)
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(Dchannel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
#endif
    // WiFi.printDiag(Serial); // Uncomment to verify channel change after
  }
  
	m_ready = esp_now_init() == 0 &&
#if defined(ARDUINO_ARCH_ESP8266)
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO) == 0 &&
#endif
	esp_now_register_send_cb(reinterpret_cast<esp_now_send_cb_t>(EspNowMQClass::sendHandler)) == 0 &&
	esp_now_register_recv_cb(reinterpret_cast<esp_now_recv_cb_t>(EspNowMQClass::recvHandler)) == 0
  ;

#if defined(ARDUINO_ARCH_ESP8266)
    wifi_get_macaddr(STATION_IF, EspNowMQ.mac); // mode
#elif defined(ARDUINO_ARCH_ESP32)
    esp_wifi_get_mac(WIFI_IF_AP, EspNowMQ.mac);
#endif
	return m_ready;
}

void EspNowMQClass::sendHandler(const uint8_t *addr, uint8_t status) {
	EspNowMQ.sending = false;
	EspNowMQ.duration_s = micros() - EspNowMQ.sendTime;
  DEBUG_PRINT("S:: " + String(EspNowMQ.duration_s) + " µs\n");
	EspNowMQ.m_txRes = status == 0 ? EspNowMQSendStatus::OK : EspNowMQSendStatus::FAIL;
#ifdef MQ_DEBUG_ALL_MSG
  DEBUG_PRINT("Message to "+macStr((uint8_t *)addr) +"\n");
#endif
};
