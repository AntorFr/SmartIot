#pragma once

#include <AsyncMqttClient.h>
#include "../Logger.hpp"
#include "../Blinker.hpp"
#include "../LoopFunction.hpp"
#include "../Constants.hpp"
#include "../Config.hpp"
#include "../Limits.hpp"
#include "./Callbacks.hpp"
#include "../../SmartIotBootMode.hpp"
#include "../../SmartIotNode.hpp"
#include "../../SendingPromise.hpp"
#include "../../SmartIotEvent.hpp"

namespace SmartIotInternals {
class Logger;
class Blinker;
class Config;
class LoopFunction;
class SendingPromise;
class SmartIotClass;

class InterfaceData {
  friend SmartIotClass;

 public:
  InterfaceData();

  /***** User configurable data *****/
  char brand[MAX_BRAND_LENGTH];

  SmartIotBootMode bootMode;

  struct ConfigurationAP {
    bool secured;
    char password[MAX_WIFI_PASSWORD_LENGTH];
  } configurationAp;

  struct Firmware {
    char name[MAX_FIRMWARE_NAME_LENGTH];
    char version[MAX_FIRMWARE_VERSION_LENGTH];
  } firmware;

  struct LED {
    bool enabled;
    uint8_t pin;
    uint8_t on;
  } led;

  struct Reset {
    bool enabled;
    bool idle;
    uint8_t triggerPin;
    uint8_t triggerState;
    uint16_t triggerTime;
    bool resetFlag;
  } reset;



  bool disable;
  bool flaggedForSleep;

  GlobalInputHandler globalInputHandler;
  BroadcastHandler broadcastHandler;
  OperationFunction setupFunction;
  //OperationFunction loopFunction;
  EventHandler eventHandler;

  /***** Runtime data *****/
  SmartIotEvent event;
  bool ready;
  Logger& getLogger() { return *_logger; }
  Blinker& getBlinker() { return *_blinker; }
  Config& getConfig() { return *_config; }
  AsyncMqttClient& getMqttClient() { return *_mqttClient; }
  SendingPromise& getSendingPromise() { return *_sendingPromise; }
  LoopFunction& getLoop() { return *_loopFunction; }

 private:
  Logger* _logger;
  Blinker* _blinker;
  Config* _config;
  AsyncMqttClient* _mqttClient;
  SendingPromise* _sendingPromise;
  LoopFunction* _loopFunction;
};

class Interface {
 public:
  static InterfaceData& get();

 private:
  static InterfaceData _interface;
};
}  // namespace SmartIotInternals
