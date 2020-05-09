
#pragma once

#include "Arduino.h"

#include <functional>
#include <libb64/cdecode.h>

#ifndef SMARTIOT_MDNS
#define SMARTIOT_MDNS 1
#endif


#ifdef ESP32
#include <WiFi.h>
#include <Update.h>
#if SMARTIOT_MDNS
#include <ESPmDNS.h>
#endif
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#if SMARTIOT_MDNS
#include <ESP8266mDNS.h>
#endif
#endif // ESP32


#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include "../../SmartIotNode.hpp"
#include "../../SmartIotRange.hpp"
#include "../../StreamingOperator.hpp"
#include "../Constants.hpp"
#include "../Limits.hpp"
#include "../Datatypes/Interface.hpp"
#include "../Utils/Helpers.hpp"
#include "../Uptime.hpp"
#include "../Timer.hpp"
#include "../ExponentialBackoffTimer.hpp"
#include "Boot.hpp"
#include "../Utils/ResetHandler.hpp"

namespace SmartIotInternals {
class BootNormal : public Boot {
 public:
  BootNormal();
  ~BootNormal();
  void setup();
  void loop();

 private:
  struct AdvertisementProgress {
    bool done = false;
    enum class GlobalStep {
      PUB_INIT,
      SUB_SMARTIOT,
      SUB_NODES,
      PUB_READY
    } globalStep;

    size_t currentNodeIndex;
    size_t currentArrayNodeIndex;
    size_t currentPropertyIndex;
  } _advertisementProgress;
  Uptime _uptime;
  Timer _statsTimer;
  ExponentialBackoffTimer _mqttReconnectTimer;
  bool _setupFunctionCalled;
  #ifdef ESP32
  WiFiEventId_t _wifiGotIpHandler;
  WiFiEventId_t _wifiDisconnectedHandler;
  #elif defined(ESP8266)
  WiFiEventHandler _wifiGotIpHandler;
  WiFiEventHandler _wifiDisconnectedHandler;
  #endif // ESP32
  bool _mqttConnectNotified;
  bool _mqttDisconnectNotified;
  bool _otaOngoing;
  bool _flaggedForReboot;
  uint16_t _mqttOfflineMessageId;
  char _fwChecksum[32 + 1];
  bool _otaIsBase64;
  base64_decodestate _otaBase64State;
  size_t _otaBase64Pads;
  size_t _otaSizeTotal;
  size_t _otaSizeDone;

  std::unique_ptr<char[]> _mqttTopic;
  std::unique_ptr<char[]> _jsonMessageBuffer;
  std::unique_ptr<char[]> _mqttClientId;
  std::unique_ptr<char[]> _mqttWillTopic;
  std::unique_ptr<char[]> _mqttPayloadBuffer;
  std::unique_ptr<char*[]> _mqttTopicLevels;
  uint8_t _mqttTopicLevelsCount;
  std::unique_ptr<char[]> _mqttTopicCopy;

  void _wifiConnect();
  #ifdef ESP32
  void _onWifiGotIp(WiFiEvent_t event, WiFiEventInfo_t info);
  void _onWifiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
  #elif defined(ESP8266)
  void _onWifiGotIp(const WiFiEventStationModeGotIP& event);
  void _onWifiDisconnected(const WiFiEventStationModeDisconnected& event);
  #endif // ESP32
  void _mqttConnect();
  void _advertise();
  bool _subscribe();
  void _onMqttConnected();
  void _onMqttDisconnected(AsyncMqttClientDisconnectReason reason);
  void _onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
  void _onMqttPublish(uint16_t id);
  void _prefixMqttTopic();
  char* _firmwareMqttTopic(PGM_P topic);
  char* _prefixMqttTopic(PGM_P topic,bool set = false);
  char* _deviceMqttTopic(PGM_P topic,bool set = false);
  char* _nodeMqttTopic(SmartIotNode* nodeIndex,bool set= false);
  void _publish_stats();
  bool _publishOtaStatus(int status, const char* info = nullptr);
  void _endOtaUpdate(bool success, uint8_t update_error = UPDATE_ERROR_OK);

  // _onMqttMessage Helpers
  void __splitTopic(char* topic);
  bool __fillPayloadBuffer(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleOTAUpdates(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleBroadcasts(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleResets(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleConfig(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleNodeProperty(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleNode(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
};
}  // namespace SmartIotInternals
