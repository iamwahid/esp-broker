#include "espnow_mq.h"
#include "helpers.h"
#include "objects.h"
#include <string.h>
#include <map>

#if defined(ARDUINO_ARCH_ESP8266)
#include <c_types.h>
#include <espnow.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <esp_now.h>
#include <esp_wifi.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

using namespace espbroker;

esp_rc_event_t rc_events[250];

EspNowMQClass EspNowMQ;

MessageObject txMsg;


// ESPNOWMQ
EspNowMQClass::EspNowMQClass() = default;

void EspNowMQClass::setMaster(uint8_t *addr) {
	if (esp_now_is_peer_exist(this->master))
		esp_now_del_peer(this->master);
	this->master = addr;

  this->addPeer(this->master);
}

bool EspNowMQClass::send(const uint8_t mac[6], const uint8_t* buf, size_t count)
{
  if (!this->m_ready || count > ESPNOW_MQ_MAXMSGLEN || count == 0) {
    return false;
  }

  this->sending = true;
  this->sendTime = micros();

  DEBUG_PRINT("sending...\n");
  this->m_txRes = EspNowMQSendStatus::NONE;
  return esp_now_send(const_cast<uint8_t*>(mac), const_cast<uint8_t*>(buf),
                      static_cast<int>(count)) == 0;
}

bool EspNowMQClass::send(const String type, const uint8_t dst[6], const String name, void *value) {
  if (this->m_ready) {
    txMsg.m_event_name = type;
    memcpy(txMsg.dst, dst, 6);
#if defined(ARDUINO_ARCH_ESP8266)
    wifi_get_macaddr(STATION_IF, txMsg.src); // mode
#elif defined(ARDUINO_ARCH_ESP32)
    esp_wifi_get_mac(WIFI_IF_AP, txMsg.src);
#endif

    txMsg.data.name = name;
    txMsg.data.setInt(*((double *) value));

    if (sizeof(txMsg) > ESPNOW_MQ_MAXMSGLEN || sizeof(txMsg) == 0) {
      DEBUG_PRINT("Size Exceeded\n");
      return false;
    }

    this->sending = true;
    this->sendTime = micros();

    return esp_now_send(const_cast<uint8_t*>(dst), (uint8_t *) &txMsg, sizeof(txMsg)) == 0;
  }
  DEBUG_PRINT("Send Failed\n");
  return false;
}

void EspNowMQClass::wait() {
	while (this->sending) yield();  // Wait while sending
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

int EspNowMQClass::listPeers(WifiNowPeer* peers, int maxPeers) const
{
	if (!this->m_ready) {
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
  if (this->m_ready) {
    this->listenerCount = std::min(this->listPeers(this->peerListeners, this->MAX_PEERS), this->MAX_PEERS);
    return this->listenerCount > 0;
  }
  return false;
}

void EspNowMQClass::updatePeer(WifiNowPeer * peers, const uint8_t mac[6], String listenOn)
{
  if (this->m_ready && this->hasPeer(mac)) {
    for (int i = 0; i < this->listenerCount; i++)
    {
       if (this->equals(peers[i].mac, mac, 6)) {
         peers[i].listenOn = listenOn;
         DEBUG_PRINT(listenOn+"\n");
         break;
       }
    }
  }
}
		
void EspNowMQClass::updatePeer(WifiNowPeer * peers, const uint8_t mac[6], ListenerType type)
{
  if (this->m_ready && this->hasPeer(mac)) {
    for (int i = 0; i < this->listenerCount; i++)
    {
       if (this->equals(peers[i].mac, mac, 6)) {
         peers[i].type = type;
         break;
       }
    }
  }
}

#if defined(ARDUINO_ARCH_ESP8266)
bool EspNowMQClass::addPeer(const uint8_t mac[6], int channel, const uint8_t key[ESPNOW_MQ_KEYLEN])
{
  if (!this->m_ready) {
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
  if (!this->m_ready) {
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
  if (this->Dchannel) 
    pi.channel = this->Dchannel;
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
  return this->m_ready && esp_now_is_peer_exist(const_cast<uint8_t*>(mac));
}

int EspNowMQClass::indexOf(const uint8_t mac[6])
{
  if (this->hasPeer(mac)) {
    for (int i = 0; i < this->listenerCount; i++)
    {
       if (this->equals(this->peerListeners[i].mac, mac, 6)) {
        return i;
       }
    }
  }
  return -1;
}

bool EspNowMQClass::removePeer(const uint8_t mac[6])
{
  return this->m_ready && esp_now_del_peer(const_cast<uint8_t*>(mac)) == 0;
}

void EspNowMQClass::onReceive(esp_rx_cb_t cb, void* arg)
{
  this->m_rxCb = cb;
  this->m_rxArg = arg;
}

void EspNowMQClass::end()
{
  if (!this->m_ready) {
    return;
  }
  esp_now_deinit();
  this->m_ready = false;
}

bool EspNowMQClass::begin(uint8_t role, const char * SSID) {
	this->end();


  if (role == NET_ROLE::BROKER) {
    this->Dchannel = getWiFiChannel(SSID);
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("ESPNOW_MQ_NET", nullptr, 3);
    // WiFi.softAPdisconnect(false); // for unassign AP name
  } else if (role == NET_ROLE::LISTENER) {
    this->Dchannel = getWiFiChannel("ESPNOW_MQ_NET");
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // WiFi.printDiag(Serial); // Uncomment to verify channel number before
#if defined(ARDUINO_ARCH_ESP8266)
    wifi_promiscuous_enable(1);
    wifi_set_channel(this->Dchannel);
    wifi_promiscuous_enable(0);
#elif defined(ARDUINO_ARCH_ESP32)
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(this->Dchannel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
#endif
    // WiFi.printDiag(Serial); // Uncomment to verify channel change after
  } 
  else {

  }
  
	this->m_ready = esp_now_init() == 0 &&
#if defined(ARDUINO_ARCH_ESP8266)
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO) == 0 &&
#endif
	esp_now_register_send_cb(reinterpret_cast<esp_now_send_cb_t>(EspNowMQClass::sendHandler)) == 0 &&
	esp_now_register_recv_cb(reinterpret_cast<esp_now_recv_cb_t>(EspNowMQClass::recvHandler)) == 0
  ;

#if defined(ARDUINO_ARCH_ESP8266)
    wifi_get_macaddr(STATION_IF, this->mac); // mode
#elif defined(ARDUINO_ARCH_ESP32)
    esp_wifi_get_mac(WIFI_IF_AP, this->mac);
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
