#include "BootStandalone.hpp"

using namespace SmartIotInternals;


BootStandalone::BootStandalone()
  : Boot("standalone")
  , _mqttReconnectTimer(MQTT_RECONNECT_INITIAL_INTERVAL, MQTT_RECONNECT_MAX_BACKOFF)
  , _mqttConnectNotified(false)
  , _mqttDisconnectNotified(true)
  , _otaOngoing(false)
  , _flaggedForReboot(false)
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

BootStandalone::~BootStandalone() {
}

void BootStandalone::setup() {
  Boot::setup();

  strlcpy(_fwChecksum, ESP.getSketchMD5().c_str(), sizeof(_fwChecksum));
  _fwChecksum[sizeof(_fwChecksum) - 1] = '\0';

  #ifdef ESP32
  //FIXME
  #elif defined(ESP8266)
  Update.runAsync(true);
  #endif // ESP32

  _statsTimer.setInterval(STATS_SEND_INTERVAL_SEC * 1000);

  if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);

  // Generate topic buffer
  size_t baseTopicLength = strlen(DEFAULT_MQTT_BASE_TOPIC) + strlen(DeviceId::getChipId());
  size_t longestSubtopicLength = 70 + 1;  // /$implementation/ota/firmware/$hash+
  
  _mqttTopic = std::unique_ptr<char[]>(new char[baseTopicLength + longestSubtopicLength]);
  _jsonMessageBuffer = std::unique_ptr<char[]>(new char[JSON_MSG_BUFFER]);

  #ifdef ESP32
  _wifiGotIpHandler = WiFi.onEvent(std::bind(&BootStandalone::_onWifiGotIp, this, std::placeholders::_1, std::placeholders::_2), WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
  _wifiDisconnectedHandler = WiFi.onEvent(std::bind(&BootStandalone::_onWifiDisconnected, this, std::placeholders::_1, std::placeholders::_2), WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
  #elif defined(ESP8266)
  _wifiGotIpHandler = WiFi.onStationModeGotIP(std::bind(&BootStandalone::_onWifiGotIp, this, std::placeholders::_1));
  _wifiDisconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&BootStandalone::_onWifiDisconnected, this, std::placeholders::_1));
  #endif // ESP32

  Interface::get().getMqttClient().onConnect(std::bind(&BootStandalone::_onMqttConnected, this));
  Interface::get().getMqttClient().onDisconnect(std::bind(&BootStandalone::_onMqttDisconnected, this, std::placeholders::_1));
  Interface::get().getMqttClient().onMessage(std::bind(&BootStandalone::_onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

  Interface::get().getMqttClient().setServer(MQTT_SERVER, DEFAULT_MQTT_PORT);
  Interface::get().getMqttClient().setMaxTopicLength(MAX_MQTT_TOPIC_LENGTH);

  _mqttClientId = std::unique_ptr<char[]>(new char[strlen(Interface::get().brand) + 1 + strlen(DeviceId::getChipId()) + 1]);
  strcpy(_mqttClientId.get(), Interface::get().brand);
  strcat_P(_mqttClientId.get(), PSTR("-"));
  strcat(_mqttClientId.get(), DeviceId::getChipId());
  Interface::get().getMqttClient().setClientId(_mqttClientId.get());
  char* mqttWillTopic = _deviceMqttTopic(PSTR("log/state"));
  _mqttWillTopic = std::unique_ptr<char[]>(new char[strlen(mqttWillTopic) + 1]);
  memcpy(_mqttWillTopic.get(), mqttWillTopic, strlen(mqttWillTopic) + 1);
  Interface::get().getMqttClient().setWill(_mqttWillTopic.get(), 1, true, "lost");

#ifdef DEBUG
  Interface::get().getLogger() << F("✔ MQTT lastWill topic: ") << _mqttWillTopic.get() << endl;
#endif

  if (MQTT_AUTH) Interface::get().getMqttClient().setCredentials(MQTT_USER, MQTT_PSWD);

#if SMARTIOT_CONFIG
  ResetHandler::Attach();
#endif
  
  //Config NTP
  Interface::get().getTime().init();

  _wifiConnect();
}

void BootStandalone::loop() {
  Boot::loop();

  if (_flaggedForReboot && Interface::get().reset.idle) {
    Interface::get().getLogger() << F("Device is idle") << endl;
    Interface::get().disable = true;
    Interface::get().getLogger() << F("↻ Rebooting...") << endl;
    Serial.flush();
    ESP.restart();
  }

  if(_mqttReconnectTimer.isActive()){
    if (_mqttReconnectTimer.check()) {
      Interface::get().getLogger() << F("↕ _mqttReconnectTimer up ...") << endl;
      _mqttConnect();
      return;
    } 
    if (_mqttReconnectTimer.reachMax()){
      Interface::get().getLogger() << F("↕ too many mqtt connect attent > reset device ...") << endl;
      _flaggedForReboot = true;
    }
  }

  if (!Interface::get().getMqttClient().connected()) return;
  // here, we are connected to the broker
  
  if (_otaOngoing) return; // if OTA ongoing do noting

  if (!_mqttConnectNotified) {
    _subscribe();
    Interface::get().ready = true;
    if (Interface::get().led.enabled) Interface::get().getBlinker().stop();
    Interface::get().getLogger() << F("✔ MQTT ready") << endl;
    _mqttConnectNotified = true;
    return;
  }

  if (_statsTimer.check()) {
    _publish_stats();
  }
}

void BootStandalone::_publish_stats(){
    DynamicJsonDocument jsonBuffer (JSON_OBJECT_SIZE(10)); 
    JsonObject statsData = jsonBuffer.to<JsonObject>();

    Interface::get().getLogger() << F("〽 Sending statistics...") << endl;

    char statsIntervalStr[3 + 1];
    itoa(Interface::get().getConfig().get().deviceStatsInterval+5, statsIntervalStr, 10);
    Interface::get().getLogger() << F("  • Interval: ") << statsIntervalStr << F("s (") << STATS_SEND_INTERVAL_SEC << F("s including 5s grace time)") << endl;
    statsData[F("stats_interval")] = STATS_SEND_INTERVAL_SEC+5;

    uint8_t quality = Helpers::rssiToPercentage(WiFi.RSSI());
    char qualityStr[3 + 1];
    itoa(quality, qualityStr, 10);
    Interface::get().getLogger() << F("  • Wi-Fi signal quality: ") << qualityStr << F("%") << endl;   
    statsData[F("wifi_quality")] = quality;

    Interface::get().getUpTime().update();
    uint64_t upsec = Interface::get().getUpTime().getSeconds();

    char uptimeStr[20 + 1];
    itoa(upsec, uptimeStr, 10);
    Interface::get().getLogger() << F("  • Uptime: ") << uptimeStr << F("s") << endl;
    statsData[F("uptime")] = static_cast<unsigned long> (upsec);

    if(Interface::get().getTime().isReady()){ //NTP synched
      Interface::get().getLogger() << F("  • Boot Date: ") << Interface::get().getTime().getIsoBootTime() << endl;
      statsData[F("boot_date")] = Interface::get().getTime().getBootTime();
    }

    uint32_t freeMem= ESP.getFreeHeap();
    char freeMemStr[20 + 1];
    itoa(freeMem, freeMemStr, 10);
    Interface::get().getLogger() << F("  • Free heap memory : ") << freeMemStr << endl;
    statsData[F("freeMem")] = freeMem;

    uint32_t freeBlock= ESP.getMaxFreeBlockSize();
    char freeBlockStr[20 + 1];
    itoa(freeBlock, freeBlockStr, 10);
    Interface::get().getLogger() << F("  • Free block size memory : ") << freeBlockStr << endl;
    statsData[F("freeBlock")] = freeBlock;

    serializeJson(statsData,(char*) _jsonMessageBuffer.get(),JSON_MSG_BUFFER);

    uint16_t statsPacketId = Interface::get().getMqttClient().publish(_deviceMqttTopic(PSTR("log/heartbeat")), 1, false, _jsonMessageBuffer.get());
    Interface::get().getLogger() << F("  Stats published at: ") << _mqttTopic.get() << endl;
    if (statsPacketId != 0) _statsTimer.tick();
}

char* BootStandalone::_deviceMqttTopic(PGM_P topic,bool set) {
  strcpy(_mqttTopic.get(), DEFAULT_MQTT_BASE_TOPIC);
  strcat_P(_mqttTopic.get(), topic);
  strcat_P(_mqttTopic.get(), PSTR("/"));
  strcat(_mqttTopic.get(), DeviceId::getChipId());
  if (set) strcat_P(_mqttTopic.get(), PSTR("/set"));
  return _mqttTopic.get();
}

char* BootStandalone::_firmwareMqttTopic(PGM_P topic) {
  _prefixMqttTopic();
  strcat_P(_mqttTopic.get(), topic);
  strcat_P(_mqttTopic.get(), PSTR("/"));
  strcat(_mqttTopic.get(), DeviceId::getChipId());
  strcat_P(_mqttTopic.get(), PSTR("/firmware"));
  return _mqttTopic.get();
}

void BootStandalone::_prefixMqttTopic() {
  strcpy(_mqttTopic.get(), DEFAULT_MQTT_BASE_TOPIC);
}

char* BootStandalone::_prefixMqttTopic(PGM_P topic,bool set) {
  _prefixMqttTopic();
  strcat_P(_mqttTopic.get(), topic);
  if (set) strcat_P(_mqttTopic.get(), PSTR("/set"));
  return _mqttTopic.get();
}

bool BootStandalone::_publishOtaStatus(uint32_t status, const char* info) {
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

void BootStandalone::_endOtaUpdate(bool success, uint8_t update_error) {
  if (success) {
    Interface::get().getLogger() << F("✔ OTA succeeded") << endl;

    _publishOtaStatus(200);  // 200 OK
    _flaggedForReboot = true;
  } else {
    Update.end();
    uint8_t code;
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
  }
  _otaOngoing = false;
}

void BootStandalone::_wifiConnect() {
  if (!Interface::get().disable) {
    if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);
    Interface::get().getLogger() << F("↕ Attempting to connect to Wi-Fi...") << endl;

    if (WiFi.getMode() != WIFI_STA) WiFi.mode(WIFI_STA);

    #ifdef ESP32
    WiFi.setHostname(DeviceId::getChipId());
    #elif defined(ESP8266)
    WiFi.hostname(DeviceId::getChipId());
    #endif // ESP32

    WiFi.begin(WIFI_SSID,WIFI_PSWD);

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
void BootStandalone::_onWifiGotIp(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (Interface::get().led.enabled) Interface::get().getBlinker().stop();
  Interface::get().getLogger() << F("✔ Wi-Fi connected, IP: ") << IPAddress(info.got_ip.ip_info.ip.addr) << endl;
  Interface::get().getLogger() << F("Triggering WIFI_CONNECTED event...") << endl;
  Interface::get().event.type = SmartIotEventType::WIFI_CONNECTED;
  Interface::get().event.ip = IPAddress(info.got_ip.ip_info.ip.addr);
  Interface::get().event.mask = IPAddress(info.got_ip.ip_info.netmask.addr);
  Interface::get().event.gateway = IPAddress(info.got_ip.ip_info.gw.addr);
  Interface::get().eventHandler(Interface::get().event);
  MDNS.begin(DeviceId::getChipId());

  _mqttConnect();
}
#elif defined(ESP8266)
void BootStandalone::_onWifiGotIp(const WiFiEventStationModeGotIP& event) {
  if (Interface::get().led.enabled) Interface::get().getBlinker().stop();
  Interface::get().getLogger() << F("✔ Wi-Fi connected, IP: ") << event.ip << endl;
  Interface::get().getLogger() << F("Triggering WIFI_CONNECTED event...") << endl;
  Interface::get().event.type = SmartIotEventType::WIFI_CONNECTED;
  Interface::get().event.ip = event.ip;
  Interface::get().event.mask = event.mask;
  Interface::get().event.gateway = event.gw;
  Interface::get().eventHandler(Interface::get().event);
#if SMARTIOT_MDNS
  MDNS.begin(DeviceId::getChipId());
#endif

  _mqttConnect();
}
#endif // ESP32

#ifdef ESP32
void BootStandalone::_onWifiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
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
void BootStandalone::_onWifiDisconnected(const WiFiEventStationModeDisconnected& event) {
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

void BootStandalone::_mqttConnect() {
  if (!Interface::get().disable) {
    if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_MQTT_DELAY);
    _mqttConnectNotified = false;
    Interface::get().getMqttClient().disconnect();
    Interface::get().getLogger() << F("↕ Attempting to connect to MQTT...") << endl;
    Interface::get().getMqttClient().connect();
  }
}

bool BootStandalone::_subscribe(){
  uint16_t packetId;
  String baseTopic_topic(DEFAULT_MQTT_BASE_TOPIC);
  String device_id_topic(DeviceId::getChipId());

  Interface::get().getLogger() << F(" 〽 Start subscribing to SmartIoT MQTT topics: ") << endl;

  //setup device command
  packetId = Interface::get().getMqttClient().subscribe(_deviceMqttTopic(PSTR("setup/+"),true), 1);
  Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;
  if (packetId == 0) return false;

  //reset OTA command
  packetId = Interface::get().getMqttClient().subscribe(_firmwareMqttTopic(PSTR("setup/ota")), 1);
  Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;
  if (packetId == 0) return false;

  return true;
}

void BootStandalone::_onMqttConnected() {
  _mqttDisconnectNotified = false;
  _mqttReconnectTimer.deactivate();
  _statsTimer.activate();

  Interface::get().getMqttClient().publish(_mqttWillTopic.get(), 1, true, "connected");
}

void BootStandalone::_onMqttDisconnected(AsyncMqttClientDisconnectReason reason) {
  Interface::get().ready = false;
  _mqttConnectNotified = false;
  if (!_mqttDisconnectNotified) {
    _statsTimer.deactivate();
    Interface::get().getLogger() << F("✖ MQTT disconnected, reason: ") << (int8_t)reason << endl;
    _mqttDisconnectNotified = true;

    if(_otaOngoing){
      //OTA was on going, stop it...
      _endOtaUpdate(false,99);
    }

    _mqttConnect();
  }
  _mqttReconnectTimer.activate();
}

void BootStandalone::_onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
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

  // 6. handle config set
  if (__handleConfig(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;
}

// _onMqttMessage Helpers

void BootStandalone::__splitTopic(char* topic) {
  // split topic on each "/"
  char* afterBaseTopic = topic + strlen(DEFAULT_MQTT_BASE_TOPIC);

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


bool SmartIotInternals::BootStandalone::__fillPayloadBuffer(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
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

bool SmartIotInternals::BootStandalone::__handleOTAUpdates(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  if(
    _mqttTopicLevelsCount ==  4 // it only a firmware info.
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("setup")) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("ota")) == 0
    && strcmp(_mqttTopicLevels.get()[2], DeviceId::getChipId()) == 0
    && strcmp_P(_mqttTopicLevels.get()[3], PSTR("firmware")) == 0
    ){
    Interface::get().getLogger() << F("Receiving OTA info") << endl;

    DynamicJsonDocument otaJson (JSON_OBJECT_SIZE(9)); 
    DeserializationError error = deserializeJson(otaJson, payload);
    if (error) {
      Interface::get().getLogger() << F("✖ Bad OTA data format: ") << error.c_str() << endl;
      return true;
    }

    const char* fw_name = otaJson[F("name")];
    const char* fw_version = otaJson[F("version")];
    const char* fw_ota = otaJson[F("md5")];

    Interface::get().getLogger() << F("> New firware  received: ") << fw_name << F(":") << fw_version << F(" subscrib to it") << endl;
    _prefixMqttTopic();
    strcat_P(_mqttTopic.get(), PSTR("log/ota/"));
    strcat(_mqttTopic.get(), fw_name);
    strcat_P(_mqttTopic.get(), PSTR("/firmware/"));
    strcat(_mqttTopic.get(), fw_ota);
    Interface::get().getMqttClient().subscribe(_mqttTopic.get(), 1);
    Interface::get().getLogger() << F("  ✔ ") << _mqttTopic.get() << endl;


    return true;
  }
  else if (
    _mqttTopicLevelsCount ==  5
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("setup")) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("ota")) == 0
    // level 3 is fw name
    && strcmp_P(_mqttTopicLevels.get()[3], PSTR("firmware")) == 0
    ) {
    if (index == 0) {
      Interface::get().getLogger() << F("Receiving OTA payload") << endl;

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
        //write_len = (size_t)base64_decode_block(payload, dec_len, &c, &_otaBase64State);
        write_len = static_cast<size_t>(base64_decode_block(payload, dec_len, &c, &_otaBase64State));
        *payload = c;

        if (bin_len > 1) {
          //write_len += (size_t)base64_decode_block((const char*)payload + dec_len, bin_len - dec_len, payload + write_len, &_otaBase64State);
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

        static uint32_t count = 0;
        if (count == 100) {
          _publishOtaStatus(206, progress.c_str());  // 206 Partial Content
          count = 0;
        }
        ++count;

        Interface::get().getLogger() << F("Receiving OTA firmware (") << progress << F(")...") << endl;

        Interface::get().event.type = SmartIotEventType::OTA_PROGRESS;
        Interface::get().event.sizeDone = _otaSizeDone;
        Interface::get().event.sizeTotal = _otaSizeTotal;
        Interface::get().eventHandler(Interface::get().event);

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

bool SmartIotInternals::BootStandalone::__handleConfig(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  if (
    _mqttTopicLevelsCount >= 4
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("setup")) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("config")) == 0
    && strcmp(_mqttTopicLevels.get()[2], DeviceId::getChipId()) == 0
    && strcmp_P(_mqttTopicLevels.get()[3], PSTR("set")) == 0 
    ) {

    bool loading_sucess = Interface::get().getConfig().write(_mqttPayloadBuffer.get());
    Interface::get().getMqttClient().publish(topic, 1, true, "");

    if (loading_sucess) {
      Interface::get().getLogger() << F("✔ Configuration created") << endl;
      Interface::get().getMqttClient().publish(_deviceMqttTopic(PSTR("/log/info")), 1, false, "Configuration created" );
      _flaggedForReboot = true;
      Interface::get().getLogger() << F("Flagged for reboot") << endl;
    } else {
      Interface::get().getLogger() << F("✖ Configuration not updated") << endl;
    }

    return true;
  }
  return false;
}
