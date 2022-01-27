#include "BootNormal.hpp"

using namespace SmartIotInternals;

BootNormal::BootNormal()
  : Boot("normal")
  , _mqttReconnectTimer(MQTT_RECONNECT_INITIAL_INTERVAL, MQTT_RECONNECT_MAX_BACKOFF)
  , _setupFunctionCalled(false)
  , _mqttConnectNotified(false)
  , _mqttDisconnectNotified(true)
  , _otaOngoing(false)
  , _flaggedForReboot(false)
  , _mqttOfflineMessageId(0)
  , _otaIsBase64(false)
  , _otaBase64Pads(0)
  , _otaSizeTotal(0)
  , _otaSizeDone(0)
  , _mqttTopic(nullptr)
  , _jsonMessageBuffer(nullptr)
  , _mqttClientId(nullptr)
  , _mqttWillTopic(nullptr)
  , _mqttPayloadBuffer(nullptr)
  , _mqttTopicLevels(nullptr)
  , _mqttTopicLevelsCount(0)
  , _mqttTopicCopy(nullptr) {
}

BootNormal::~BootNormal() {
}

void BootNormal::setup() {
  Boot::setup();

  strlcpy(_fwChecksum, ESP.getSketchMD5().c_str(), sizeof(_fwChecksum));
  _fwChecksum[sizeof(_fwChecksum) - 1] = '\0';

  #ifdef ESP32
  //FIXME
  #elif defined(ESP8266)
  Update.runAsync(true);
  #endif // ESP32

  _statsTimer.setInterval(Interface::get().getConfig().get().deviceStatsInterval * 1000);

  if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);

  // Generate topic buffer
  size_t baseTopicLength = strlen(Interface::get().getConfig().get().mqtt.baseTopic) + strlen(Interface::get().getConfig().get().deviceId);
  size_t longestSubtopicLength = 70 + 1;  // /$implementation/ota/firmware/$hash+
  for (SmartIotNode* iNode : SmartIotNode::nodes) {
    size_t nodeMaxTopicLength = 1 + strlen(iNode->getId()) + 12 + 1;  // /id/$properties
    if (nodeMaxTopicLength > longestSubtopicLength) longestSubtopicLength = nodeMaxTopicLength;

    for (Property* iProperty : iNode->getProperties()) {
      size_t propertyMaxTopicLength = 1 + strlen(iNode->getId()) + 1 + strlen(iProperty->getId()) + 1;
      if (iProperty->isSettable()) propertyMaxTopicLength += 4;  // /set

      if (propertyMaxTopicLength > longestSubtopicLength) longestSubtopicLength = propertyMaxTopicLength;
    }
  }
  _mqttTopic = std::unique_ptr<char[]>(new char[baseTopicLength + longestSubtopicLength]);
  _jsonMessageBuffer = std::unique_ptr<char[]>(new char[JSON_MSG_BUFFER]);

  #ifdef ESP32
  _wifiGotIpHandler = WiFi.onEvent(std::bind(&BootNormal::_onWifiGotIp, this, std::placeholders::_1, std::placeholders::_2), WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
  _wifiDisconnectedHandler = WiFi.onEvent(std::bind(&BootNormal::_onWifiDisconnected, this, std::placeholders::_1, std::placeholders::_2), WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
  #elif defined(ESP8266)
  _wifiGotIpHandler = WiFi.onStationModeGotIP(std::bind(&BootNormal::_onWifiGotIp, this, std::placeholders::_1));
  _wifiDisconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&BootNormal::_onWifiDisconnected, this, std::placeholders::_1));
  #endif // ESP32

  Interface::get().getMqttClient().onConnect(std::bind(&BootNormal::_onMqttConnected, this));
  Interface::get().getMqttClient().onDisconnect(std::bind(&BootNormal::_onMqttDisconnected, this, std::placeholders::_1));
  Interface::get().getMqttClient().onMessage(std::bind(&BootNormal::_onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  Interface::get().getMqttClient().onPublish(std::bind(&BootNormal::_onMqttPublish, this, std::placeholders::_1));

  Interface::get().getMqttClient().setServer(Interface::get().getConfig().get().mqtt.server.host, Interface::get().getConfig().get().mqtt.server.port);

#if ASYNC_TCP_SSL_ENABLED
  Interface::get().getLogger() << "SSL is: " << Interface::get().getConfig().get().mqtt.server.ssl.enabled << endl;
  Interface::get().getMqttClient().setSecure(Interface::get().getConfig().get().mqtt.server.ssl.enabled);
  if (Interface::get().getConfig().get().mqtt.server.ssl.enabled && Interface::get().getConfig().get().mqtt.server.ssl.hasFingerprint) {
    char hexBuf[MAX_FINGERPRINT_STRING_LENGTH];
    Helpers::byteArrayToHexString(Interface::get().getConfig().get().mqtt.server.ssl.fingerprint, hexBuf, MAX_FINGERPRINT_SIZE);
    Interface::get().getLogger() << "Using fingerprint: " << hexBuf << endl;
    Interface::get().getMqttClient().addServerFingerprint((const uint8_t*)Interface::get().getConfig().get().mqtt.server.ssl.fingerprint);
  }
#endif

  Interface::get().getMqttClient().setMaxTopicLength(MAX_MQTT_TOPIC_LENGTH);
  _mqttClientId = std::unique_ptr<char[]>(new char[strlen(Interface::get().brand) + 1 + strlen(Interface::get().getConfig().get().deviceId) + 1]);
  strcpy(_mqttClientId.get(), Interface::get().brand);
  strcat_P(_mqttClientId.get(), PSTR("-"));
  strcat(_mqttClientId.get(), Interface::get().getConfig().get().deviceId);
  Interface::get().getMqttClient().setClientId(_mqttClientId.get());
  char* mqttWillTopic = _deviceMqttTopic(PSTR("log/state"));
  _mqttWillTopic = std::unique_ptr<char[]>(new char[strlen(mqttWillTopic) + 1]);
  memcpy(_mqttWillTopic.get(), mqttWillTopic, strlen(mqttWillTopic) + 1);
  Interface::get().getMqttClient().setWill(_mqttWillTopic.get(), 1, true, "lost");

#ifdef DEBUG
  Interface::get().getLogger() << F("✔ MQTT lastWill topic: ") << _mqttWillTopic.get() << endl;
#endif

  if (Interface::get().getConfig().get().mqtt.auth) Interface::get().getMqttClient().setCredentials(Interface::get().getConfig().get().mqtt.username, Interface::get().getConfig().get().mqtt.password);

#if SMARTIOT_CONFIG
  ResetHandler::Attach();
#endif
  //Config NTP
  Interface::get().getTime().init();

  Interface::get().getConfig().log();

  for (SmartIotNode* iNode : SmartIotNode::nodes) {
    iNode->setup();
  }

  _wifiConnect();
}

void BootNormal::loop() {
  Boot::loop();

  if (_flaggedForReboot && Interface::get().reset.idle) {
    Interface::get().getLogger() << F("Device is idle") << endl;
    Interface::get().disable = true;
    Interface::get().getLogger() << F("↻ Rebooting...") << endl;
    Serial.flush();
    ESP.restart();
  }

  if(_mqttReconnectTimer.isActive()){
    if (_mqttReconnectTimer.reachMax()){
      Interface::get().getLogger() << F("↕ too many mqtt connect attent > reset device ...") << endl;
      _flaggedForReboot = true;
      return;
    }
    if (_mqttReconnectTimer.check()) {
      Interface::get().getLogger() << F("↕ _mqttReconnectTimer up ...") << endl;
      _mqttConnect();
      return;
    } 
  }

  for (SmartIotNode* iNode : SmartIotNode::nodes) {
      if (!_otaOngoing && (iNode->runLoopDisconnected)) iNode->loop();
  }

  if (!Interface::get().getMqttClient().connected()) return;
  // here, we are connected to the broker
  
  if (_otaOngoing) return; // if OTA ongoing do noting

  if (!_advertisementProgress.done) {
    _advertise();
    return;
  }

  // here, we finished the advertisement

  if (!_mqttConnectNotified) {
    Interface::get().ready = true;
    if (Interface::get().led.enabled) Interface::get().getBlinker().stop();

    Interface::get().getLogger() << F("✔ MQTT ready") << endl;
    Interface::get().getLogger() << F("Triggering MQTT_READY event...") << endl;
    Interface::get().event.type = SmartIotEventType::MQTT_READY;
    Interface::get().eventHandler(Interface::get().event);

    for (SmartIotNode* iNode : SmartIotNode::nodes) {
      iNode->onReadyToOperate();
    }

    if (!_setupFunctionCalled) {
      Interface::get().getLogger() << F("Calling setup function...") << endl;
      Interface::get().setupFunction();
      _setupFunctionCalled = true;
    }

    _mqttConnectNotified = true;
    return;
  }

  if (_mqttConnectNotified){
    for (SmartIotNode* iNode : SmartIotNode::nodes) {
      if (!(iNode->runLoopDisconnected)) iNode->loop();
    }
  }

  // here, we have notified the sketch we are ready

  if (_mqttOfflineMessageId == 0 && Interface::get().flaggedForSleep) {
    Interface::get().getLogger() << F("Device in preparation to sleep...") << endl;
    _mqttOfflineMessageId = Interface::get().getMqttClient().publish(_mqttWillTopic.get(), 1, true, "sleeping");
  }

  if (_statsTimer.check()) {
    _publish_stats();
  }

  Interface::get().getLoop().run();
}

void BootNormal::_publish_stats(){
    DynamicJsonDocument jsonBuffer (JSON_OBJECT_SIZE(7)); 
    JsonObject statsData = jsonBuffer.to<JsonObject>();

    Interface::get().getLogger() << F("〽 Sending statistics...") << endl;

    char statsIntervalStr[3 + 1];
    itoa(Interface::get().getConfig().get().deviceStatsInterval+5, statsIntervalStr, 10);
    Interface::get().getLogger() << F("  • Interval: ") << statsIntervalStr << F("s (") << Interface::get().getConfig().get().deviceStatsInterval << F("s including 5s grace time)") << endl;
    statsData[F("stats_interval")] = Interface::get().getConfig().get().deviceStatsInterval+5;

    uint8_t quality = Helpers::rssiToPercentage(WiFi.RSSI());
    char qualityStr[3 + 1];
    itoa(quality, qualityStr, 10);
    Interface::get().getLogger() << F("  • Wi-Fi signal quality: ") << qualityStr << F("%") << endl;   
    statsData[F("wifi_quality")] = quality;

    _uptime.update();
    char uptimeStr[20 + 1];
    itoa(_uptime.getSeconds(), uptimeStr, 10);
    Interface::get().getLogger() << F("  • Uptime: ") << uptimeStr << F("s") << endl;
    statsData[F("uptime")] = static_cast<unsigned long> (_uptime.getSeconds());

    uint32_t freeMem= ESP.getFreeHeap();
    char freeMemStr[20 + 1];
    itoa(ESP.getFreeHeap(), freeMemStr, 10);
    Interface::get().getLogger() << F("  • Free heap memory : ") << freeMemStr << endl;
    statsData[F("freeMem")] = freeMem;

    serializeJson(statsData,(char*) _jsonMessageBuffer.get(),JSON_MSG_BUFFER);

    uint16_t statsPacketId = Interface::get().getMqttClient().publish(_deviceMqttTopic(PSTR("log/heartbeat")), 1, false, _jsonMessageBuffer.get());
    
    if (statsPacketId != 0) {
      _statsTimer.tick();
      Interface::get().getLogger() << F("  Stats Published at: ") << _mqttTopic.get() << endl;
      Interface::get().event.type = SmartIotEventType::SENDING_STATISTICS;
      Interface::get().eventHandler(Interface::get().event);
    } else {
      Interface::get().getLogger() << F("  Stats Published at: ") << _mqttTopic.get() << endl;
      Interface::get().getLogger() << F("✖ impossible to publish stat") << endl;
    }

    for (SmartIotNode* iNode : SmartIotNode::nodes) {
      iNode->publish_stats();
    }
}

char* BootNormal::_deviceMqttTopic(PGM_P topic,bool set) {
  strcpy(_mqttTopic.get(), Interface::get().getConfig().get().mqtt.baseTopic);
  strcat_P(_mqttTopic.get(), topic);
  strcat_P(_mqttTopic.get(), PSTR("/"));
  strcat(_mqttTopic.get(), Interface::get().getConfig().get().deviceId);
  if (set) strcat_P(_mqttTopic.get(), PSTR("/set"));
  return _mqttTopic.get();
}

char* BootNormal::_nodeMqttTopic(SmartIotNode* node,bool set) {
  _prefixMqttTopic();
  strcat(_mqttTopic.get(), node->getType());
  strcat_P(_mqttTopic.get(), PSTR("/"));
  strcat(_mqttTopic.get(), node->getName());
  if (set) strcat_P(_mqttTopic.get(), PSTR("/set"));
  return _mqttTopic.get();
}

char* BootNormal::_nodePropertyMqttTopic(SmartIotNode* node,Property* property,bool set) {
  _prefixMqttTopic();
  strcat(_mqttTopic.get(), node->getType());
  strcat_P(_mqttTopic.get(), PSTR("/"));
  strcat(_mqttTopic.get(), node->getName());
  strcat_P(_mqttTopic.get(), PSTR("/"));
  strcat(_mqttTopic.get(), property->getName());
  if (set) strcat_P(_mqttTopic.get(), PSTR("/set"));
  return _mqttTopic.get();
}

char* BootNormal::_firmwareMqttTopic(PGM_P topic) {
  _prefixMqttTopic();
  strcat_P(_mqttTopic.get(), topic);
  strcat_P(_mqttTopic.get(), PSTR("/"));
  strcat(_mqttTopic.get(), Interface::get().firmware.name);
  strcat_P(_mqttTopic.get(), PSTR("/firmware"));
  return _mqttTopic.get();
}

void BootNormal::_prefixMqttTopic() {
  strcpy(_mqttTopic.get(), Interface::get().getConfig().get().mqtt.baseTopic);
}

char* BootNormal::_prefixMqttTopic(PGM_P topic,bool set) {
  _prefixMqttTopic();
  strcat_P(_mqttTopic.get(), topic);
  if (set) strcat_P(_mqttTopic.get(), PSTR("/set"));
  return _mqttTopic.get();
}

bool BootNormal::_publishOtaStatus(uint32_t status, const char* info) {
  DynamicJsonDocument jsonBuffer (JSON_OBJECT_SIZE(3));
  JsonObject data = jsonBuffer.to<JsonObject>();
  data["status"]  = status;
  if (info) {
    data["info"]  = info;
  }
  serializeJson(data,(char*) _jsonMessageBuffer.get(),JSON_MSG_BUFFER);

  return Interface::get().getMqttClient().publish(
            _deviceMqttTopic(PSTR("log/ota")), 0, false, _jsonMessageBuffer.get()) != 0;
}

void BootNormal::_endOtaUpdate(bool success, uint8_t update_error) {
  if (success) {
    Interface::get().getLogger() << F("✔ OTA succeeded") << endl;
    Interface::get().getLogger() << F("Triggering OTA_SUCCESSFUL event...") << endl;
    Interface::get().event.type = SmartIotEventType::OTA_SUCCESSFUL;
    Interface::get().eventHandler(Interface::get().event);
    _publishOtaStatus(200);  // 200 OK
    _flaggedForReboot = true;
  } else {
    Update.end();
    uint16_t code;
    String info;
    switch (update_error) {
      case UPDATE_ERROR_SIZE:               // new firmware size is zero
      case UPDATE_ERROR_MAGIC_BYTE:         // new firmware does not have 0xE9 in first byte
      #ifdef ESP32
      //FIXME
      #elif defined(ESP8266)
      case UPDATE_ERROR_NEW_FLASH_CONFIG:   // bad new flash config (does not match flash ID)
        code = 400;  // 400 Bad Request
        info.concat(F("BAD_FIRMWARE"));
        break;
      #endif //ESP32
      case UPDATE_ERROR_MD5:
        code = 400;  // 400 Bad Request
        info.concat(F("BAD_CHECKSUM"));
        break;
      case UPDATE_ERROR_SPACE:
        code = 400;  // 400 Bad Request
        info.concat(F("NOT_ENOUGH_SPACE"));
        break;
      case UPDATE_ERROR_WRITE:
      case UPDATE_ERROR_ERASE:
      case UPDATE_ERROR_READ:
        code = 500;  // 500 Internal Server Error
        info.concat(F("FLASH_ERROR"));
        break;
      case 99:
        code = 500;  // 500 Internal Server Error
        info.concat(F("MQTT DISCONNECT"));
        break;       
      default:
        code = 500;  // 500 Internal Server Error
        info.concat(F("INTERNAL_ERROR "));
        info.concat(update_error);
        break;
    }
    _publishOtaStatus(code, info.c_str());

    Interface::get().getLogger() << F("✖ OTA failed (") << code << F(" ") << info << F(")") << endl;

    Interface::get().getLogger() << F("Triggering OTA_FAILED event...") << endl;
    Interface::get().event.type = SmartIotEventType::OTA_FAILED;
    Interface::get().eventHandler(Interface::get().event);
    _flaggedForReboot = true;
  }
  _otaOngoing = false;
}

void BootNormal::_wifiConnect() {
  if (!Interface::get().disable) {
    if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);
    Interface::get().getLogger() << F("↕ Attempting to connect to Wi-Fi...") << endl;

    if (WiFi.getMode() != WIFI_STA) WiFi.mode(WIFI_STA);

    #ifdef ESP32
    WiFi.setHostname(Interface::get().getConfig().get().deviceId);
    #elif defined(ESP8266)
    WiFi.hostname(Interface::get().getConfig().get().deviceId);
    #endif // ESP32
    if (strcmp_P(Interface::get().getConfig().get().wifi.ip, PSTR("")) != 0) {  // on _validateConfigWifi there is a requirement for mask and gateway
      IPAddress convertedIp;
      convertedIp.fromString(Interface::get().getConfig().get().wifi.ip);
      IPAddress convertedMask;
      convertedMask.fromString(Interface::get().getConfig().get().wifi.mask);
      IPAddress convertedGateway;
      convertedGateway.fromString(Interface::get().getConfig().get().wifi.gw);

      if (strcmp_P(Interface::get().getConfig().get().wifi.dns1, PSTR("")) != 0) {
        IPAddress convertedDns1;
        convertedDns1.fromString(Interface::get().getConfig().get().wifi.dns1);
        if ((strcmp_P(Interface::get().getConfig().get().wifi.dns2, PSTR("")) != 0)) {  // on _validateConfigWifi there is requirement that we need dns1 if we want to define dns2
          IPAddress convertedDns2;
          convertedDns2.fromString(Interface::get().getConfig().get().wifi.dns2);
          WiFi.config(convertedIp, convertedGateway, convertedMask, convertedDns1, convertedDns2);
        } else {
          WiFi.config(convertedIp, convertedGateway, convertedMask, convertedDns1);
        }
      } else {
        WiFi.config(convertedIp, convertedGateway, convertedMask);
      }
    }

    if (strcmp_P(Interface::get().getConfig().get().wifi.bssid, PSTR("")) != 0) {
      byte bssidBytes[6];
      Helpers::stringToBytes(Interface::get().getConfig().get().wifi.bssid, ':', bssidBytes, 6, 16);
      WiFi.begin(Interface::get().getConfig().get().wifi.ssid, Interface::get().getConfig().get().wifi.password, Interface::get().getConfig().get().wifi.channel, bssidBytes);
    } else {
      WiFi.begin(Interface::get().getConfig().get().wifi.ssid, Interface::get().getConfig().get().wifi.password);
    }

    #ifdef ESP32
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    #elif defined(ESP8266)
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    #endif // ESP32
  }
}

#ifdef ESP32
void BootNormal::_onWifiGotIp(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (Interface::get().led.enabled) Interface::get().getBlinker().stop();
  Interface::get().getLogger() << F("✔ Wi-Fi connected, IP: ") << IPAddress(info.got_ip.ip_info.ip.addr) << endl;
  Interface::get().getLogger() << F("Triggering WIFI_CONNECTED event...") << endl;
  Interface::get().event.type = SmartIotEventType::WIFI_CONNECTED;
  Interface::get().event.ip = IPAddress(info.got_ip.ip_info.ip.addr);
  Interface::get().event.mask = IPAddress(info.got_ip.ip_info.netmask.addr);
  Interface::get().event.gateway = IPAddress(info.got_ip.ip_info.gw.addr);
  Interface::get().eventHandler(Interface::get().event);
  MDNS.begin(Interface::get().getConfig().get().deviceId);

  _mqttConnect();
}
#elif defined(ESP8266)
void BootNormal::_onWifiGotIp(const WiFiEventStationModeGotIP& event) {
  if (Interface::get().led.enabled) Interface::get().getBlinker().stop();
  Interface::get().getLogger() << F("✔ Wi-Fi connected, IP: ") << event.ip << endl;
  Interface::get().getLogger() << F("Triggering WIFI_CONNECTED event...") << endl;
  Interface::get().event.type = SmartIotEventType::WIFI_CONNECTED;
  Interface::get().event.ip = event.ip;
  Interface::get().event.mask = event.mask;
  Interface::get().event.gateway = event.gw;
  Interface::get().eventHandler(Interface::get().event);
#if SMARTIOT_MDNS
  MDNS.begin(Interface::get().getConfig().get().deviceId);
#endif

  _mqttConnect();
}
#endif // ESP32

#ifdef ESP32
void BootNormal::_onWifiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Interface::get().ready = false;
  if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);
  _statsTimer.deactivate();
  Interface::get().getLogger() << F("✖ Wi-Fi disconnected, reason: ") << info.disconnected.reason << endl;
  Interface::get().getLogger() << F("Triggering WIFI_DISCONNECTED event...") << endl;
  Interface::get().event.type = SmartIotEventType::WIFI_DISCONNECTED;
  Interface::get().event.wifiReason = info.disconnected.reason;
  Interface::get().eventHandler(Interface::get().event);

  _wifiConnect();
}
#elif defined(ESP8266)
void BootNormal::_onWifiDisconnected(const WiFiEventStationModeDisconnected& event) {
  Interface::get().ready = false;
  if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);
  _statsTimer.deactivate();
  Interface::get().getLogger() << F("✖ Wi-Fi disconnected, reason: ") << event.reason << endl;
  Interface::get().getLogger() << F("Triggering WIFI_DISCONNECTED event...") << endl;
  Interface::get().event.type = SmartIotEventType::WIFI_DISCONNECTED;
  Interface::get().event.wifiReason = event.reason;
  Interface::get().eventHandler(Interface::get().event);

  _wifiConnect();
}
#endif // ESP32

void BootNormal::_mqttConnect() {
  if (!Interface::get().disable) {
    if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_MQTT_DELAY);
    _mqttConnectNotified = false;
    Interface::get().getMqttClient().disconnect();
    Interface::get().getLogger() << F("↕ Attempting to connect to MQTT...") << endl;
    Interface::get().getMqttClient().connect();
  }
}

bool BootNormal::_publish_config() {
  uint16_t packetId;
  Interface::get().getLogger() << F(" > sending  config...") << endl;
  packetId = Interface::get().getMqttClient().publish(_deviceMqttTopic(PSTR("log/config")), 1, true, Interface::get().getConfig().getSafeConfigFile());
  return (packetId != 0); 
}


bool BootNormal::_publish_advertise() {
  auto numSettings = ISmartIotSetting::settings.size();
  auto numNodes = SmartIotNode::nodes.size();
  DynamicJsonDocument jsonBuffer (
    JSON_OBJECT_SIZE(6) + //data
    JSON_OBJECT_SIZE(6) + //stats
    JSON_OBJECT_SIZE(6) + //fw
    JSON_OBJECT_SIZE(6) + //implementation
    JSON_ARRAY_SIZE(numNodes) + // Array "nodes"
    (JSON_OBJECT_SIZE(3) ) * numNodes + // Objects in "nodes"
    JSON_ARRAY_SIZE(numSettings) + // Array "settings"
    numSettings * JSON_OBJECT_SIZE(3)); // Objects in "settings";
  JsonObject data = jsonBuffer.to<JsonObject>();
  data[F("name")]= Interface::get().getConfig().get().name;
  data[F("id")]=  Interface::get().getConfig().get().deviceId;
  data[F("mac")]= WiFi.macAddress();

  IPAddress localIp = WiFi.localIP();
  char localIpStr[MAX_IP_STRING_LENGTH];
  Helpers::ipToString(localIp, localIpStr);
  data[F("ip")]= localIpStr;
  JsonObject stats = data.createNestedObject("stats");
  stats[F("stats_interval")] = Interface::get().getConfig().get().deviceStatsInterval+5;  
  uint8_t quality = Helpers::rssiToPercentage(WiFi.RSSI());
  stats[F("wifi_quality")] = quality;
  _uptime.update();
  stats[F("uptime")] = static_cast<unsigned long> (_uptime.getSeconds());
  uint32_t freeMem= ESP.getFreeHeap();
  stats[F("freeMem")] = freeMem;
  uint16_t packetId;

  JsonObject fw = data.createNestedObject("fw");
  fw[F("name")]= Interface::get().firmware.name;
  fw[F("version")]= Interface::get().firmware.version;
  fw[F("checksum")]= _fwChecksum;

  JsonObject implementation = data.createNestedObject("implementation");
  #ifdef ESP32
    implementation[F("device")]= "esp32";
  #elif defined(ESP8266)
    implementation[F("device")]= "esp8266";  
  #endif // ESP32
  implementation[F("ota")]=Interface::get().getConfig().get().ota.enabled;
  implementation[F("version")]=SMARTIOT_VERSION;
  if (SmartIotNode::nodes.size()) {
    JsonArray nodesData = data.createNestedArray("nodes");
    for (SmartIotNode* node : SmartIotNode::nodes) {
      JsonObject nodeData = nodesData.createNestedObject();
      nodeData["id"]=node->getId();
      nodeData["name"]=node->getName();
      nodeData["type"]=node->getType();

      if (node->getProperties().size()) {
        JsonArray nodesPropertiesData = nodeData.createNestedArray("nodes");
        for (Property* iProperty : node->getProperties()) {
            JsonObject nodesPropertieData = nodesPropertiesData.createNestedObject();
            nodesPropertieData[F("id")]= iProperty->getId();
            if (iProperty->getName() && (iProperty->getName()[0] != '\0')) {
              nodesPropertieData[F("name")]= iProperty->getName();
            }
        }
      }
    }
  }

  serializeJson(data,(char*) _jsonMessageBuffer.get(),JSON_MSG_BUFFER);
  Interface::get().getLogger() << F(" 〽 sending  advertise...") << endl;
  packetId = Interface::get().getMqttClient().publish(_deviceMqttTopic(PSTR("log/advertise")), 1, true, _jsonMessageBuffer.get());
  return (packetId != 0); 
}

void BootNormal::_advertise() {
  switch (_advertisementProgress.globalStep) {
    case AdvertisementProgress::GlobalStep::PUB_INIT:
    {
      Interface::get().getLogger() << F("〽 Start  advertise...") << endl;
      if (_publish_advertise()) {
        _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::SUB_SMARTIOT;
      } 
      break;
    }
    case AdvertisementProgress::GlobalStep::SUB_SMARTIOT:
    case AdvertisementProgress::GlobalStep::SUB_NODES:
    {
      _subscribe();
      break;
    }
    case AdvertisementProgress::GlobalStep::PUB_READY:
    {
      _advertisementProgress.done = true;
      Interface::get().getLogger() << F("✔ advertise sent.") << endl;
      break;
    }
  }
}

bool BootNormal::_subscribe(){
  uint16_t packetId;
  String baseTopic_topic(Interface::get().getConfig().get().mqtt.baseTopic);
  String device_id_topic(Interface::get().getConfig().get().deviceId);
  switch (_advertisementProgress.globalStep) {
    case AdvertisementProgress::GlobalStep::SUB_SMARTIOT:
    {
    Interface::get().getLogger() << F(" 〽 Start subscribing to SmartIoT MQTT topics: ") << endl;

    //heartbeat broadcast command 
    packetId = Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("log/+"),true), 1);
    Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;
    if (packetId == 0) return false;

    //heartbeat device command
    packetId = Interface::get().getMqttClient().subscribe(_deviceMqttTopic(PSTR("log/+"),true), 1);
    Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;
    if (packetId == 0) return false;

    //setup broadcast command
    packetId = Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("setup/+"),true), 1);
    Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;
    if (packetId == 0) return false;
  
    //setup device command
    packetId = Interface::get().getMqttClient().subscribe(_deviceMqttTopic(PSTR("setup/+"),true), 1);
    Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;
    if (packetId == 0) return false;

    //reset OTA command
    packetId = Interface::get().getMqttClient().subscribe(_firmwareMqttTopic(PSTR("setup/ota")), 1);
    Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;
    if (packetId == 0) return false;
  


    _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::SUB_NODES;
    return true;
    break;
    }
    case AdvertisementProgress::GlobalStep::SUB_NODES:
    {
      if (SmartIotNode::nodes.size()>0) {
        Interface::get().getLogger() << F(" 〽 Start subscribing to SmartIoT MQTT node topics: ") << endl;
        for (SmartIotNode* node : SmartIotNode::nodes) {
          if (node->isSettable()) {
            packetId = Interface::get().getMqttClient().subscribe(_nodeMqttTopic(node,true), 1);
            Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;
            if (packetId == 0) return false;
          }
          for (Property* iProperty : node->getProperties()) {
            if (iProperty->isSettable()) {
              packetId = Interface::get().getMqttClient().subscribe(_nodePropertyMqttTopic(node,iProperty,true), 1);
              Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;
              if (packetId == 0) return false; 
            }
          }
        }
      } 
      _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_READY;
      return true;
      break;
    }
  }
  return false;
}

void BootNormal::_onMqttConnected() {
  _mqttDisconnectNotified = false;
  _mqttReconnectTimer.deactivate();
  _statsTimer.activate();

  Interface::get().getMqttClient().publish(_mqttWillTopic.get(), 1, true, "connected");
}

void BootNormal::_onMqttDisconnected(AsyncMqttClientDisconnectReason reason) {
  Interface::get().ready = false;
  _mqttConnectNotified = false;
  _advertisementProgress.done = false;
  _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_INIT;
  _advertisementProgress.currentNodeIndex = 0;
  _advertisementProgress.currentPropertyIndex = 0;
  if (!_mqttDisconnectNotified) {
    _statsTimer.deactivate();
    Interface::get().getLogger() << F("✖ MQTT disconnected, reason: ") << (int8_t)reason << endl;
    Interface::get().getLogger() << F("Triggering MQTT_DISCONNECTED event...") << endl;
    Interface::get().event.type = SmartIotEventType::MQTT_DISCONNECTED;
    Interface::get().event.mqttReason = reason;
    Interface::get().eventHandler(Interface::get().event);

    _mqttDisconnectNotified = true;

    if (Interface::get().flaggedForSleep) {
      _mqttOfflineMessageId = 0;
      Interface::get().getLogger() << F("Triggering READY_TO_SLEEP event...") << endl;
      Interface::get().event.type = SmartIotEventType::READY_TO_SLEEP;
      Interface::get().eventHandler(Interface::get().event);

      return;
    }

    if(_otaOngoing){
      //OTA was on going, stop it...
      _endOtaUpdate(false,99);
    }

    _mqttConnect();
  }
  _mqttReconnectTimer.activate();
}

void BootNormal::_onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (total == 0) return;  // no empty message possible
  if(_flaggedForReboot && Interface::get().reset.idle) return; //don't handle message when rebooting

  if (index == 0) {
    // Copy the topic
    size_t topicLength = strlen(topic);
    _mqttTopicCopy = std::unique_ptr<char[]>(new char[topicLength+1]);
    memcpy(_mqttTopicCopy.get(), topic, topicLength);
    _mqttTopicCopy.get()[topicLength] = '\0';

    // Split the topic copy on each "/"
    __splitTopic(_mqttTopicCopy.get());
  }

  // 1. Handle OTA firmware (not copied to payload buffer)
  if (__handleOTAUpdates(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  // 2. Fill Payload Buffer
  if (__fillPayloadBuffer(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  /* Arrived here, the payload is complete */

  // 3. handle broadcasts TODO> not used anymore
  //if (__handleBroadcasts(_mqttTopicCopy.get(), payload, properties, len, index, total))
  //  return;

  // 5. handle reset
  if (__handleResets(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  // 5. handle reset
  if (__handleHeartbeat(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  // 5. handle advertise
  if (__handleAdvertise(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;


  // 4.all following messages are only for this deviceId
  //if (strcmp(_mqttTopicLevels.get()[0], Interface::get().getConfig().get().deviceId) != 0)
  //  return;

  // 6. handle config set
  if (__handleConfig(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  // 7. handle node channel
  if (__handleNode(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  // 7. here, we're sure we have a node property
  if (__handleNodeProperty(_mqttTopicCopy.get(), payload, properties, len, index, total))
   return;
}

void BootNormal::_onMqttPublish(uint16_t id) {
  Interface::get().event.type = SmartIotEventType::MQTT_PACKET_ACKNOWLEDGED;
  Interface::get().event.packetId = id;
  Interface::get().eventHandler(Interface::get().event);

  if (Interface::get().flaggedForSleep && id == _mqttOfflineMessageId) {
    Interface::get().getLogger() << F("Offline message acknowledged. Disconnecting MQTT...") << endl;
    Interface::get().getMqttClient().disconnect();
  }
}

// _onMqttMessage Helpers

void BootNormal::__splitTopic(char* topic) {
  // split topic on each "/"
  char* afterBaseTopic = topic + strlen(Interface::get().getConfig().get().mqtt.baseTopic);

  uint8_t topicLevelsCount = 1;
  for (uint8_t i = 0; i < strlen(afterBaseTopic); i++) {
    if (afterBaseTopic[i] == '/') topicLevelsCount++;
  }

  _mqttTopicLevels = std::unique_ptr<char*[]>(new char*[topicLevelsCount]);
  _mqttTopicLevelsCount = topicLevelsCount;

  const char* delimiter = "/";
  uint8_t topicLevelIndex = 0;

  char* token = strtok(afterBaseTopic, delimiter);
  while (token != nullptr) {
    _mqttTopicLevels[topicLevelIndex++] = token;

    token = strtok(nullptr, delimiter);
  }
}

bool SmartIotInternals::BootNormal::__fillPayloadBuffer(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  // Reallocate Buffer everytime a new message is received
  if (_mqttPayloadBuffer == nullptr || index == 0) _mqttPayloadBuffer = std::unique_ptr<char[]>(new char[total + 1]);

  // copy payload into buffer
  memcpy(_mqttPayloadBuffer.get() + index, payload, len);

  // return if payload buffer is not complete
  if (index + len != total)
    return true;
  // terminate buffer
  _mqttPayloadBuffer.get()[total] = '\0';
  return false;
}

bool SmartIotInternals::BootNormal::__handleOTAUpdates(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  if(
    _mqttTopicLevelsCount ==  4 // it only a firmware info.
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("setup")) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("ota")) == 0
    && strcmp(_mqttTopicLevels.get()[2], Interface::get().firmware.name) == 0
    && strcmp_P(_mqttTopicLevels.get()[3], PSTR("firmware")) == 0
    ){
    Interface::get().getLogger() << F("Receiving OTA info") << endl;
    if (!Interface::get().getConfig().get().ota.enabled) {
      Interface::get().getLogger() << F("✖ Ignore it, OTA not enabled") << endl;
      return true;
    } else {
      DynamicJsonDocument otaJson (JSON_OBJECT_SIZE(9)); 
      DeserializationError error = deserializeJson(otaJson, payload);
      if (error) {
        Interface::get().getLogger() << F("✖ Bad OTA data format: ") << error.c_str() << endl;
        return true;
      }
      const char* fw_version = otaJson[F("version")];
      const char* fw_ota = otaJson[F("md5")];
      if(strcmp(fw_version, Interface::get().firmware.version) != 0) {
        Interface::get().getLogger() << F("> New firware version received: ") << fw_version << F(" subscrib to it") << endl;
        _firmwareMqttTopic(PSTR("setup/ota"));
        strcat_P(_mqttTopic.get(), PSTR("/"));
        strcat(_mqttTopic.get(), fw_ota);
        Interface::get().getMqttClient().subscribe(_mqttTopic.get(), 1);
        Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;

      } else {
        Interface::get().getLogger() << F("✖ Ignore it, already uptodate") << endl;
      }
      return true;    
    }
  }
  else if (
    _mqttTopicLevelsCount ==  5
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("setup")) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("ota")) == 0
    && strcmp(_mqttTopicLevels.get()[2], Interface::get().firmware.name) == 0
    && strcmp_P(_mqttTopicLevels.get()[3], PSTR("firmware")) == 0
    ) {
    if (index == 0) {
      Interface::get().getLogger() << F("Receiving OTA payload") << endl;
      if (!Interface::get().getConfig().get().ota.enabled) {
        _publishOtaStatus(403);  // 403 Forbidden
        Interface::get().getLogger() << F("✖ Aborting, OTA not enabled") << endl;
        return true;
      }

      char* firmwareMd5 = _mqttTopicLevels.get()[4];
      if (!Helpers::validateMd5(firmwareMd5)) {
        _endOtaUpdate(false, UPDATE_ERROR_MD5);
        Interface::get().getLogger() << F("✖ Aborting, invalid MD5") << endl;
        return true;
      } else if (strcmp(firmwareMd5, _fwChecksum) == 0) {
        _publishOtaStatus(304);  // 304 Not Modified
        Interface::get().getLogger() << F("✖ Aborting, firmware is the same") << endl;
        return true;
      } else {
        Update.setMD5(firmwareMd5);
        _publishOtaStatus(202);
        _otaOngoing = true;

        //Stop all process to avoid disturbance
        Interface::get().getLoop().stop();
        for (SmartIotNode* iNode : SmartIotNode::nodes) {iNode->stop();}
        

        Interface::get().getLogger() << F("↕ OTA started") << endl;
        Interface::get().getLogger() << F("Triggering OTA_STARTED event...") << endl;
        Interface::get().event.type = SmartIotEventType::OTA_STARTED;
        Interface::get().eventHandler(Interface::get().event);
      }
    } else if (!_otaOngoing) {
      return true; // we've not validated the checksum
    }

    // here, we need to flash the payload

    if (index == 0) {
      // Autodetect if firmware is binary or base64-encoded. ESP firmware always has a magic first byte 0xE9.
      if (*payload == 0xE9) {
        _otaIsBase64 = false;
        Interface::get().getLogger() << F("Firmware is binary") << endl;
      } else {
        // Base64-decode first two bytes. Compare decoded value against magic byte.
        char plain[2];  // need 12 bits
        base64_init_decodestate(&_otaBase64State);
        int l = base64_decode_block(payload, 2, plain, &_otaBase64State);
        if ((l == 1) && (plain[0] == 0xE9)) {
          _otaIsBase64 = true;
          _otaBase64Pads = 0;
          Interface::get().getLogger() << F("Firmware is base64-encoded") << endl;
          if (total % 4) {
            // Base64 encoded length not a multiple of 4 bytes
            _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
            return true;
          }

          // Restart base64-decoder
          base64_init_decodestate(&_otaBase64State);
        } else {
          // Bad firmware format
          _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
          return true;
        }
      }
      _otaSizeDone = 0;
      _otaSizeTotal = _otaIsBase64 ? base64_decode_expected_len(total) : total;

      bool success = Update.begin(_otaSizeTotal);
      if (!success) {
        // Detected error during begin (e.g. size == 0 or size > space)
        _endOtaUpdate(false, Update.getError());
        return true;
      }
    }

    size_t write_len;
    if (_otaIsBase64) {
      // Base64-firmware: Make sure there are no non-base64 characters in the payload.
      // libb64/cdecode.c doesn't ignore such characters if the compiler treats `char`
      // as `unsigned char`.
      size_t bin_len = 0;
      char* p = payload;
      for (size_t i = 0; i < len; i++) {
        char c = *p++;
        bool b64 = ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '+') || (c == '/');
        if (b64) {
          bin_len++;
        } else if (c == '=') {
          // Ignore "=" padding (but only at the end and only up to 2)
          if (index + i < total - 2) {
            _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
            return true;
          }
          // Note the number of pad characters at the end
          _otaBase64Pads++;
        } else {
          // Non-base64 character in firmware
          _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
          return true;
        }
      }
      if (bin_len > 0) {
        // Decode base64 payload in-place. base64_decode_block() can decode in-place,
        // except for the first two base64-characters which make one binary byte plus
        // 4 extra bits (saved in _otaBase64State). So we "manually" decode the first
        // two characters into a temporary buffer and manually merge that back into
        // the payload. This one is a little tricky, but it saves us from having to
        // dynamically allocate some 800 bytes of memory for every payload chunk.
        size_t dec_len = bin_len > 1 ? 2 : 1;
        char c;
        write_len = static_cast<size_t>(base64_decode_block(payload, dec_len, &c, &_otaBase64State));
        *payload = c;

        if (bin_len > 1) {
          write_len += static_cast<size_t>(base64_decode_block((const char*)payload + dec_len, bin_len - dec_len, payload + write_len, &_otaBase64State));
        }
      } else {
        write_len = 0;
      }
    } else {
      // Binary firmware
      write_len = len;
    }
    if (write_len > 0) {
      bool success = Update.write(reinterpret_cast<uint8_t*>(payload), write_len) > 0;
      if (success) {
        // Flash write successful.
        _otaSizeDone += write_len;
        if (_otaIsBase64 && (index + len == total)) {
          // Having received the last chunk of base64 encoded firmware, we can now determine
          // the real size of the binary firmware from the number of padding character ("="):
          // If we have received 1 pad character, real firmware size modulo 3 was 2.
          // If we have received 2 pad characters, real firmware size modulo 3 was 1.
          // Correct the total firmware length accordingly.
          _otaSizeTotal -= _otaBase64Pads;
        }

        String progress(_otaSizeDone);
        progress.concat(F("/"));
        progress.concat(_otaSizeTotal);

        Interface::get().getLogger() << F("Receiving OTA firmware (") << progress << F(")...") << endl;

        Interface::get().event.type = SmartIotEventType::OTA_PROGRESS;
        Interface::get().event.sizeDone = _otaSizeDone;
        Interface::get().event.sizeTotal = _otaSizeTotal;
        Interface::get().eventHandler(Interface::get().event);

        static uint32_t count = 0;
        if (count == 100) {
          _publishOtaStatus(206, progress.c_str());  // 206 Partial Content
          count = 0;
        }
        ++count;

        //  Done with the update?
        if (index + len == total) {
          // With base64-coded firmware, we may have provided a length off by one or two
          // to Update.begin() because the base64-coded firmware may use padding (one or
          // two "=") at the end. In case of base64, total length was adjusted above.
          // Check the real length here and ask Update::end() to skip this test.
          if ((_otaIsBase64) && (_otaSizeDone != _otaSizeTotal)) {
            _endOtaUpdate(false, UPDATE_ERROR_SIZE);
            return true;
          }
          success = Update.end(_otaIsBase64);
          _endOtaUpdate(success, Update.getError());
        }
      } else {
        // Error erasing or writing flash
        _endOtaUpdate(false, Update.getError());
      }
    }
    return true;
  }
  return false;
}

bool SmartIotInternals::BootNormal::__handleBroadcasts(char * topic, char * payload, const AsyncMqttClientMessageProperties & properties, size_t len, size_t index, size_t total) {
  if (
    _mqttTopicLevelsCount == 2
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("$broadcast")) == 0
    ) {
    String broadcastLevel(_mqttTopicLevels.get()[1]);
    Interface::get().getLogger() << F("📢 Calling broadcast handler...") << endl;
    bool handled = Interface::get().broadcastHandler(broadcastLevel, _mqttPayloadBuffer.get());
    if (!handled) {
      Interface::get().getLogger() << F("The following broadcast was not handled:") << endl;
      Interface::get().getLogger() << F("  • Level: ") << broadcastLevel << endl;
      Interface::get().getLogger() << F("  • Value: ") << _mqttPayloadBuffer.get() << endl;
    }
    return true;
  }
  return false;
}



bool SmartIotInternals::BootNormal::__handleResets(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  /*
  Interface::get().getLogger() << F("📢 test if reset") << endl;
  Interface::get().getLogger() << F(" topic") << topic << endl;
  Interface::get().getLogger() << F(" mqttTopicLevelsCount") << _mqttTopicLevelsCount << endl;
  Interface::get().getLogger() << F(" topic level 1: ") << _mqttTopicLevels.get()[0] << endl;
  Interface::get().getLogger() << F(" topic level 2: ") << _mqttTopicLevels.get()[1] << endl;
  */
  if (
    _mqttTopicLevelsCount >= 3
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("setup")) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("reset")) == 0
    ) {
    Interface::get().getMqttClient().publish(_deviceMqttTopic(PSTR("/log/info")), 1, false, "Flagged for reboot");
    _flaggedForReboot = true;
    Interface::get().getLogger() << F("Flagged for reboot") << endl;
    return true;
  }
  return false;
}


bool SmartIotInternals::BootNormal::__handleHeartbeat(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  if (
    _mqttTopicLevelsCount >= 3
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("log")) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("heartbeat")) == 0
    && (
      strcmp_P(_mqttTopicLevels.get()[2], PSTR("set")) == 0
      || (strcmp(_mqttTopicLevels.get()[2], Interface::get().getConfig().get().deviceId) == 0
          && strcmp_P(_mqttTopicLevels.get()[3], PSTR("set")) == 0)
      )
    ) {
    _publish_stats();
    return true;
  }
  return false;
}

bool SmartIotInternals::BootNormal::__handleAdvertise(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  if (
    _mqttTopicLevelsCount >= 3
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("log")) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("advertise")) == 0
    && (
      strcmp_P(_mqttTopicLevels.get()[2], PSTR("set")) == 0
      || (strcmp(_mqttTopicLevels.get()[2], Interface::get().getConfig().get().deviceId) == 0
          && strcmp_P(_mqttTopicLevels.get()[3], PSTR("set")) == 0)
      )
    ) {
    _publish_advertise();
    return true;
  } else if (    
    _mqttTopicLevelsCount >= 3
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("log")) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("config")) == 0
    && (
      strcmp_P(_mqttTopicLevels.get()[2], PSTR("set")) == 0
      || (strcmp(_mqttTopicLevels.get()[2], Interface::get().getConfig().get().deviceId) == 0
          && strcmp_P(_mqttTopicLevels.get()[3], PSTR("set")) == 0)
      )
    ) {
    _publish_config();
    return true; 
  }
  return false;
}


bool SmartIotInternals::BootNormal::__handleConfig(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  if (
    _mqttTopicLevelsCount >= 3
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("setup")) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("config")) == 0
    && (
      strcmp_P(_mqttTopicLevels.get()[2], PSTR("set")) == 0
      || (strcmp(_mqttTopicLevels.get()[2], Interface::get().getConfig().get().deviceId) == 0
          && strcmp_P(_mqttTopicLevels.get()[3], PSTR("set")) == 0)
      )
    ) {
    Interface::get().getMqttClient().publish(topic, 1, true, "");
    if (Interface::get().getConfig().patch(_mqttPayloadBuffer.get())) {
      Interface::get().getLogger() << F("✔ Configuration updated") << endl;
      Interface::get().getMqttClient().publish(_deviceMqttTopic(PSTR("/log/info")), 1, false, "Configuration updated" );
      _publish_config();
      _flaggedForReboot = true;
      Interface::get().getLogger() << F("Flagged for reboot") << endl;
    } else {
      Interface::get().getLogger() << F("✖ Configuration not updated") << endl;
    }
    return true;
  }
  return false;
}

bool SmartIotInternals::BootNormal::__handleNodeProperty(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  // initialize SmartIotRange
  SmartIotRange range;
  range.isRange = false;
  range.index = 0;

  char* property = _mqttTopicLevels.get()[2];
  char* node_name = _mqttTopicLevels.get()[1];
  char* node_type = _mqttTopicLevels.get()[0];

  //char* node = _mqttTopicLevels.get()[1];

  /*
  int16_t rangeSeparator = -1;
  for (uint16_t i = 0; i < strlen(node); i++) {
    if (node[i] == '_') {
      rangeSeparator = i;
      break;
    }
  }
  if (rangeSeparator != -1) {
    range.isRange = true;
    node[rangeSeparator] = '\0';
    char* rangeIndexStr = node + rangeSeparator + 1;
    String rangeIndexTest = String(rangeIndexStr);
    for (uint8_t i = 0; i < rangeIndexTest.length(); i++) {
      if (!isDigit(rangeIndexTest.charAt(i))) {
        Interface::get().getLogger() << F("Range index ") << rangeIndexStr << F(" is not valid") << endl;
        return true;
      }
    }
    range.index = rangeIndexTest.toInt();
  }
  */

  SmartIotNode* SmartIoTNode = nullptr;
  SmartIoTNode = SmartIotNode::find(node_name,node_type);

  if (!SmartIoTNode) {
    Interface::get().getLogger() << F("Node ") << node_type << F("/") << node_name << F(" not registered") << endl;
    return true;
  }

  #ifdef DEBUG
    Interface::get().getLogger() << F("Recived network message for ") << SmartIoTNode->getId() << endl;
  #endif // DEBUG

  Property* propertyObject = nullptr;
  for (Property* iProperty : SmartIoTNode->getProperties()) {
    if (strcmp(property, iProperty->getName()) == 0) {
      propertyObject = iProperty;
      break;
    }
  }

  if (!propertyObject || !propertyObject->isSettable()) {
    Interface::get().getLogger() << F("Node ") << node_type << F("/") << node_name << F(": ") << property << F(" property not settable") << endl;
    return true;
  }

  #ifdef DEBUG
    Interface::get().getLogger() << F("Calling global input handler...") << endl;
  #endif // DEBUG
  bool handled = Interface::get().globalInputHandler(*SmartIoTNode, String(_mqttPayloadBuffer.get()));
  if (handled) return true;

  #ifdef DEBUG
    Interface::get().getLogger() << F("Calling node input handler...") << endl;
  #endif // DEBUG
  //handled = SmartIoTNode->handleInput(String(_mqttPayloadBuffer.get()));
  //if (handled) return true;

  #ifdef DEBUG
    Interface::get().getLogger() << F("Calling property input handler...") << endl;
  #endif // DEBUG
  handled = propertyObject->getInputHandler()(range, String(_mqttPayloadBuffer.get()));

  if (!handled) {
    Interface::get().getLogger() << F("No handlers handled the following packet:") << endl;
    Interface::get().getLogger() << F("  • Node ID: ") << node_type << F("/") << node_name << endl;
    Interface::get().getLogger() << F("  • Is range? ");
    if (range.isRange) {
      Interface::get().getLogger() << F("yes (") << range.index << F(")") << endl;
    } else {
      Interface::get().getLogger() << F("no") << endl;
    }
    Interface::get().getLogger() << F("  • Property: ") << property << endl;
    Interface::get().getLogger() << F("  • Value: ") << _mqttPayloadBuffer.get() << endl;
  }

  return false;
}

bool SmartIotInternals::BootNormal::__handleNode(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {

  if (
    _mqttTopicLevelsCount >= 3
    && strcmp_P(_mqttTopicLevels.get()[2], PSTR("set")) != 0
  ) { return false; } // too much level to be node .. it's a node property

  char* node_name = _mqttTopicLevels.get()[1];
  char* node_type = _mqttTopicLevels.get()[0];

  #ifdef DEBUG
    Interface::get().getLogger() << F("Recived network message for ") << node_name << endl;
  #endif // DEBUG

  SmartIotNode* SmartIoTNode = nullptr;
  SmartIoTNode = SmartIotNode::find(node_name,node_type);

  if (!SmartIoTNode) {
    Interface::get().getLogger() << F("Node ") << node_type << F(" / ") << node_name << F(" not registered") << endl;
    return true;
  }

  #ifdef DEBUG
    Interface::get().getLogger() << F("Calling global input handler...") << endl;
  #endif // DEBUG
  bool handled = Interface::get().globalInputHandler(*SmartIoTNode, String(_mqttPayloadBuffer.get()));
  if (handled) return true;

  #ifdef DEBUG
    Interface::get().getLogger() << F("Calling node input handler...") << endl;
  #endif // DEBUG
  handled = SmartIoTNode->handleInput(String(_mqttPayloadBuffer.get()));
  if (handled) return true;

  if (!handled) {
    Interface::get().getLogger() << F("No handlers handled the following packet:") << endl;
    Interface::get().getLogger() << F("  • Node ID: ") << node_type << F(" / ") << node_name << endl;
    Interface::get().getLogger() << F("  • Value: ") << _mqttPayloadBuffer.get() << endl;
  }

  return false;
}
