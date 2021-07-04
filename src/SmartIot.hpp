#pragma once

#include <Arduino.h>
#include <AsyncMqttClient.h>


#include "SmartIot/Datatypes/Interface.hpp"
#include "SmartIot/Constants.hpp"
#include "SmartIot/Limits.hpp"
#include "SmartIot/Utils/DeviceId.hpp"
#include "SmartIot/Boot/Boot.hpp"
#include "SmartIot/Boot/BootStandalone.hpp"
#include "SmartIot/Boot/BootNormal.hpp"
#include "SmartIot/Boot/BootConfig.hpp"
#include "SmartIot/Logger.hpp"
#include "SmartIot/Config.hpp"
#include "SmartIot/Blinker.hpp"
#include "SmartIot/LoopFunction.hpp"

#include "SendingPromise.hpp"
#include "SmartIotBootMode.hpp"
#include "SmartIotEvent.hpp"
#include "SmartIotNode.hpp"
#include "SmartIotSetting.hpp"
#include "StreamingOperator.hpp"

// Define DEBUG for debug

#if defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO)
  #define _ESP "-d1pro"
#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI) || defined(ARDUINO_ESP8266_WEMOS_D1MINILITE)
  #define _ESP "-d1"
#elif defined(ARDUINO_ESP8266_SONOFF_BASIC)
  #define _ESP "-sonoffbasic"
#elif defined(ARDUINO_ESP8266_SONOFF_SV)
  #define _ESP "-sonoffsv"
#elif defined(ESP32)
  #define _ESP "-esp32"
#elif defined(ESP8266)
  #define _ESP "-esp8266"
#endif

#define SmartIot_setFirmware(name, version) const char* __FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" name _ESP "\x93\x44\x6b\xa7\x75"; const char* __FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" version "\xb0\x30\x48\xd4\x1a"; SmartIot.__setFirmware(__FLAGGED_FW_NAME, __FLAGGED_FW_VERSION);
#define SmartIot_setBrand(brand) const char* __FLAGGED_BRAND = "\xfb\x2a\xf5\x68\xc0" brand "\x6e\x2f\x0f\xeb\x2d"; SmartIot.__setBrand(__FLAGGED_BRAND);


namespace SmartIotInternals {
class SmartIotClass {
  friend class::SmartIotNode;
  friend SendingPromise;

 public:
  SmartIotClass();
  ~SmartIotClass();
  void setup();
  void loop();

  void __setFirmware(const char* name, const char* version);
  void __setBrand(const char* brand) const;

  SmartIotClass& disableLogging();
  SmartIotClass& setLoggingPrinter(Print* printer);
  SmartIotClass& disableLedFeedback();
  SmartIotClass& setLedPin(uint8_t pin, uint8_t on);
  SmartIotClass& setConfigurationApPassword(const char* password);
  SmartIotClass& setGlobalInputHandler(const GlobalInputHandler& globalInputHandler);
  SmartIotClass& setBroadcastHandler(const BroadcastHandler& broadcastHandler);
  SmartIotClass& onEvent(const EventHandler& handler);
  SmartIotClass& setResetTrigger(uint8_t pin, uint8_t state, uint16_t time);
  SmartIotClass& disableResetTrigger();
  SmartIotClass& setSetupFunction(const OperationFunction& function);
  #ifdef ESP32
  SmartIotClass& setLoopFunction(const OperationFunction& function, bool multitask = false,int freq = 0);
  SmartIotClass& setLoopFunction(const OperationFunction& function, bool multitask,float freq);
  #elif defined(ESP8266)
  SmartIotClass& setLoopFunction(const OperationFunction& function,int freq = 0);
  SmartIotClass& setLoopFunction(const OperationFunction& function,float freq);
  #endif
  SmartIotClass& setSmartIotBootMode(SmartIotBootMode bootMode);
  SmartIotClass& setSmartIotBootModeOnNextBoot(SmartIotBootMode bootMode);

  static void reset();
  void reboot();
  static void setIdle(bool idle);
  static bool isConfigured();
  static bool isConnected();
  static const ConfigStruct& getConfiguration();
  AsyncMqttClient& getMqttClient();
  Logger& getLogger();
  static void prepareToSleep();
  #ifdef ESP32
  static void doDeepSleep(uint64_t time_us = 0);
  static void doDeepSleep(gpio_num_t wakeupPin, int logicLevel);
  static void doDeepSleep(uint64_t pinMask, esp_sleep_ext1_wakeup_mode_t mode);
  #elif defined(ESP8266)
  static void doDeepSleep(uint64_t time_us = 0, RFMode mode = RF_DEFAULT);
  #endif // ESP32

 private:
  bool _setupCalled;
  bool _firmwareSet;
  Boot* _boot;
  BootStandalone _bootStandalone;
  BootNormal _bootNormal;
#if SMARTIOT_CONFIG
  BootConfig _bootConfig;
#endif
  bool _flaggedForReboot;
  SendingPromise _sendingPromise;
  Logger _logger;
  Blinker _blinker;
  Config _config;
  AsyncMqttClient _mqttClient;
  LoopFunction _loopFunction;

  void _checkBeforeSetup(const __FlashStringHelper* functionName) const;

  const char* __SMARTIOT_SIGNATURE;
};
}  // namespace SmartIotInternals

extern SmartIotInternals::SmartIotClass SmartIot;
