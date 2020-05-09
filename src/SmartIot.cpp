#include "SmartIot.hpp"

using namespace SmartIotInternals;

SmartIotClass::SmartIotClass()
  : _setupCalled(false)
  , _firmwareSet(false)
  , __SMARTIOT_SIGNATURE("\x25\x48\x4f\x4d\x49\x45\x5f\x45\x53\x50\x38\x32\x36\x36\x5f\x46\x57\x25") {
  strlcpy(Interface::get().brand, DEFAULT_BRAND, MAX_BRAND_LENGTH);
  Interface::get().bootMode = SmartIotBootMode::UNDEFINED;
  Interface::get().configurationAp.secured = false;
  #ifdef ESP32
  Interface::get().led.enabled = false;
  #ifdef LED_BUILTIN
  Interface::get().led.pin = LED_BUILTIN;
  #endif // LED_BUILTIN
  Interface::get().led.on = LOW;
  #elif defined(ESP8266)
  Interface::get().led.enabled = true;
  Interface::get().led.pin = LED_BUILTIN;
  Interface::get().led.on = LOW;
  #endif // ESP32
  Interface::get().reset.idle = true;
  Interface::get().reset.enabled = true;
  Interface::get().reset.triggerPin = DEFAULT_RESET_PIN;
  Interface::get().reset.triggerState = DEFAULT_RESET_STATE;
  Interface::get().reset.triggerTime = DEFAULT_RESET_TIME;
  Interface::get().reset.resetFlag = false;
  Interface::get().disable = false;
  Interface::get().flaggedForSleep = false;
  Interface::get().globalInputHandler = [](const SmartIotNode& node, const String& value) { return false; };
  Interface::get().broadcastHandler = [](const String& level, const String& value) { return false; };
  Interface::get().setupFunction = []() {};
  Interface::get().loopFunction = []() {};
  Interface::get().eventHandler = [](const SmartIotEvent& event) {};
  Interface::get().ready = false;
  Interface::get()._mqttClient = &_mqttClient;
  Interface::get()._sendingPromise = &_sendingPromise;
  Interface::get()._blinker = &_blinker;
  Interface::get()._logger = &_logger;
  Interface::get()._config = &_config;

  DeviceId::generate();
}

SmartIotClass::~SmartIotClass() {
}

void SmartIotClass::_checkBeforeSetup(const __FlashStringHelper* functionName) const {
  if (_setupCalled) {
    String message;
    message.concat(F("✖ "));
    message.concat(functionName);
    message.concat(F("(): has to be called before setup()"));
    Helpers::abort(message);
  }
}

void SmartIotClass::setup() {
  _setupCalled = true;

  // Check if firmware is set
  if (!_firmwareSet) {
    Helpers::abort(F("✖ Firmware name must be set before calling setup()"));
    return;  // never reached, here for clarity
  }

  // Check the max allowed setting elements
  if (ISmartIotSetting::settings.size() > MAX_CONFIG_SETTING_SIZE) {
    Helpers::abort(F("✖ Settings exceed set limit of elelement."));
    return;  // never reached, here for clarity
  }

  // Check if default settings values are valid
  bool defaultSettingsValuesValid = true;
  for (ISmartIotSetting* iSetting : ISmartIotSetting::settings) {
    if (iSetting->isBool()) {
      SmartIotSetting<bool>* setting = static_cast<SmartIotSetting<bool>*>(iSetting);
      if (!setting->isRequired() && !setting->validate(setting->get())) {
        defaultSettingsValuesValid = false;
        break;
      }
    } else if (iSetting->isLong()) {
      SmartIotSetting<long>* setting = static_cast<SmartIotSetting<long>*>(iSetting);
      if (!setting->isRequired() && !setting->validate(setting->get())) {
        defaultSettingsValuesValid = false;
        break;
      }
    } else if (iSetting->isDouble()) {
      SmartIotSetting<double>* setting = static_cast<SmartIotSetting<double>*>(iSetting);
      if (!setting->isRequired() && !setting->validate(setting->get())) {
        defaultSettingsValuesValid = false;
        break;
      }
    } else if (iSetting->isConstChar()) {
      SmartIotSetting<const char*>* setting = static_cast<SmartIotSetting<const char*>*>(iSetting);
      if (!setting->isRequired() && !setting->validate(setting->get())) {
        defaultSettingsValuesValid = false;
        break;
      }
    }
  }

  if (!defaultSettingsValuesValid) {
    Helpers::abort(F("✖ Default setting value does not pass validator test"));
    return;  // never reached, here for clarity
  }

  // boot mode set during this boot by application before SmartIot.setup()
  SmartIotBootMode _applicationSmartIotBootMode = Interface::get().bootMode;

  // boot mode set before resetting the device. If application has defined a boot mode, this will be ignored
  SmartIotBootMode _nextSmartIotBootMode = Interface::get().getConfig().getSmartIotBootModeOnNextBoot();
  if (_nextSmartIotBootMode != SmartIotBootMode::UNDEFINED) {
    Interface::get().getConfig().setSmartIotBootModeOnNextBoot(SmartIotBootMode::UNDEFINED);
  }

#if SMARTIOT_CONFIG
  SmartIotBootMode _selectedSmartIotBootMode = SmartIotBootMode::CONFIGURATION;
#else
  SmartIotBootMode _selectedSmartIotBootMode = SmartIotBootMode::NORMAL;
#endif

  // select boot mode source
  if (_applicationSmartIotBootMode != SmartIotBootMode::UNDEFINED) {
    _selectedSmartIotBootMode = _applicationSmartIotBootMode;
  } else if (_nextSmartIotBootMode != SmartIotBootMode::UNDEFINED) {
    _selectedSmartIotBootMode = _nextSmartIotBootMode;
  } else {
    _selectedSmartIotBootMode = SmartIotBootMode::NORMAL;
  }

  // validate selected mode and fallback as needed
  if (_selectedSmartIotBootMode == SmartIotBootMode::NORMAL && !Interface::get().getConfig().load()) {
#if SMARTIOT_CONFIG
    Interface::get().getLogger() << F("Configuration invalid. Using CONFIG MODE") << endl;
    _selectedSmartIotBootMode = SmartIotBootMode::CONFIGURATION;
#else
    Interface::get().getLogger() << F("Configuration invalid. CONFIG MODE is disabled.") << endl;
    ESP.restart();
#endif
  }

  // run selected mode
  if (_selectedSmartIotBootMode == SmartIotBootMode::NORMAL) {
    _boot = &_bootNormal;
    Interface::get().event.type = SmartIotEventType::NORMAL_MODE;
    Interface::get().eventHandler(Interface::get().event);
#if SMARTIOT_CONFIG
  } else if (_selectedSmartIotBootMode == SmartIotBootMode::CONFIGURATION) {
    _boot = &_bootConfig;
    Interface::get().event.type = SmartIotEventType::CONFIGURATION_MODE;
    Interface::get().eventHandler(Interface::get().event);
#endif
  } else if (_selectedSmartIotBootMode == SmartIotBootMode::STANDALONE) {
    _boot = &_bootStandalone;
    Interface::get().event.type = SmartIotEventType::STANDALONE_MODE;
    Interface::get().eventHandler(Interface::get().event);
  } else {
    Helpers::abort(F("✖ Boot mode invalid"));
    return;  // never reached, here for clarity
  }

  WiFi.disconnect(); // workaround for issue #351

  _boot->setup();
}

void SmartIotClass::loop() {
  _boot->loop();

  if (_flaggedForReboot && Interface::get().reset.idle) {
    Interface::get().getLogger() << F("Device is idle") << endl;
    Interface::get().getLogger() << F("Triggering ABOUT_TO_RESET event...") << endl;
    Interface::get().event.type = SmartIotEventType::ABOUT_TO_RESET;
    Interface::get().eventHandler(Interface::get().event);

    Interface::get().getLogger() << F("↻ Rebooting device...") << endl;
    Serial.flush();
    ESP.restart();
  }
}

SmartIotClass& SmartIotClass::disableLogging() {
  _checkBeforeSetup(F("disableLogging"));

  Interface::get().getLogger().setLogging(false);

  return *this;
}

SmartIotClass& SmartIotClass::setLoggingPrinter(Print* printer) {
  _checkBeforeSetup(F("setLoggingPrinter"));

  Interface::get().getLogger().setPrinter(printer);

  return *this;
}

SmartIotClass& SmartIotClass::disableLedFeedback() {
  _checkBeforeSetup(F("disableLedFeedback"));

  Interface::get().led.enabled = false;

  return *this;
}

SmartIotClass& SmartIotClass::setLedPin(uint8_t pin, uint8_t on) {
  _checkBeforeSetup(F("setLedPin"));

  Interface::get().led.pin = pin;
  Interface::get().led.on = on;
  Interface::get().led.enabled = true;

  return *this;
}

SmartIotClass& SmartIotClass::setConfigurationApPassword(const char* password) {
  _checkBeforeSetup(F("setConfigurationApPassword"));

  Interface::get().configurationAp.secured = true;
  strlcpy(Interface::get().configurationAp.password, password, MAX_WIFI_PASSWORD_LENGTH);
  return *this;
}

void SmartIotClass::__setFirmware(const char* name, const char* version) {
  _checkBeforeSetup(F("setFirmware"));
  if (strlen(name) + 1 - 10 > MAX_FIRMWARE_NAME_LENGTH || strlen(version) + 1 - 10 > MAX_FIRMWARE_VERSION_LENGTH) {
    Helpers::abort(F("✖ setFirmware(): either the name or version string is too long"));
    return;  // never reached, here for clarity
  }

  strncpy(Interface::get().firmware.name, name + 5, strlen(name) - 10);
  Interface::get().firmware.name[strlen(name) - 10] = '\0';
  strncpy(Interface::get().firmware.version, version + 5, strlen(version) - 10);
  Interface::get().firmware.version[strlen(version) - 10] = '\0';
  _firmwareSet = true;
}

void SmartIotClass::__setBrand(const char* brand) const {
  _checkBeforeSetup(F("setBrand"));
  if (strlen(brand) + 1 - 10 > MAX_BRAND_LENGTH) {
    Helpers::abort(F("✖ setBrand(): the brand string is too long"));
    return;  // never reached, here for clarity
  }

  strncpy(Interface::get().brand, brand + 5, strlen(brand) - 10);
  Interface::get().brand[strlen(brand) - 10] = '\0';
}

void SmartIotClass::reset() {
  Interface::get().getLogger() << F("Flagged for reset by sketch") << endl;
  Interface::get().disable = true;
  Interface::get().reset.resetFlag = true;
}

void SmartIotClass::reboot() {
  Interface::get().getLogger() << F("Flagged for reboot by sketch") << endl;
  Interface::get().disable = true;
  _flaggedForReboot = true;
}

void SmartIotClass::setIdle(bool idle) {
  Interface::get().reset.idle = idle;
}

SmartIotClass& SmartIotClass::setGlobalInputHandler(const GlobalInputHandler& globalInputHandler) {
  _checkBeforeSetup(F("setGlobalInputHandler"));

  Interface::get().globalInputHandler = globalInputHandler;

  return *this;
}

SmartIotClass& SmartIotClass::setBroadcastHandler(const BroadcastHandler& broadcastHandler) {
  _checkBeforeSetup(F("setBroadcastHandler"));

  Interface::get().broadcastHandler = broadcastHandler;

  return *this;
}

SmartIotClass& SmartIotClass::setSetupFunction(const OperationFunction& function) {
  _checkBeforeSetup(F("setSetupFunction"));

  Interface::get().setupFunction = function;

  return *this;
}

SmartIotClass& SmartIotClass::setLoopFunction(const OperationFunction& function) {
  _checkBeforeSetup(F("setLoopFunction"));

  Interface::get().loopFunction = function;

  return *this;
}

SmartIotClass& SmartIotClass::setSmartIotBootMode(SmartIotBootMode bootMode) {
  _checkBeforeSetup(F("setSmartIotBootMode"));
  Interface::get().bootMode = bootMode;
  return *this;
}

SmartIotClass& SmartIotClass::setSmartIotBootModeOnNextBoot(SmartIotBootMode bootMode) {
  Interface::get().getConfig().setSmartIotBootModeOnNextBoot(bootMode);
  return *this;
}

bool SmartIotClass::isConfigured() {
  return Interface::get().getConfig().load();
}

bool SmartIotClass::isConnected() {
  return Interface::get().ready;
}

SmartIotClass& SmartIotClass::onEvent(const EventHandler& handler) {
  _checkBeforeSetup(F("onEvent"));

  Interface::get().eventHandler = handler;

  return *this;
}

SmartIotClass& SmartIotClass::setResetTrigger(uint8_t pin, uint8_t state, uint16_t time) {
  _checkBeforeSetup(F("setResetTrigger"));

  Interface::get().reset.enabled = true;
  Interface::get().reset.triggerPin = pin;
  Interface::get().reset.triggerState = state;
  Interface::get().reset.triggerTime = time;

  return *this;
}

SmartIotClass& SmartIotClass::disableResetTrigger() {
  _checkBeforeSetup(F("disableResetTrigger"));

  Interface::get().reset.enabled = false;

  return *this;
}

const ConfigStruct& SmartIotClass::getConfiguration() {
  return Interface::get().getConfig().get();
}

AsyncMqttClient& SmartIotClass::getMqttClient() {
  return _mqttClient;
}

Logger& SmartIotClass::getLogger() {
  return _logger;
}

#ifdef ESP32
//FIXME: implement for ESP32
#elif defined(ESP8266)
void SmartIotClass::prepareToSleep() {
  Interface::get().getLogger() << F("Flagged for sleep by sketch") << endl;
  if (Interface::get().ready) {
    Interface::get().disable = true;
    Interface::get().flaggedForSleep = true;
  } else {
    Interface::get().disable = true;
    Interface::get().getLogger() << F("Triggering READY_TO_SLEEP event...") << endl;
    Interface::get().event.type = SmartIotEventType::READY_TO_SLEEP;
    Interface::get().eventHandler(Interface::get().event);
  }
}

void SmartIotClass::doDeepSleep(uint64_t time_us, RFMode mode) {
  Interface::get().getLogger() << F("💤 Device is deep sleeping...") << endl;
  Serial.flush();
  ESP.deepSleep(time_us, mode);
}
#endif // ESP32


SmartIotClass SmartIot;
