#pragma once

#include "Arduino.h"

#include <ArduinoJson.h>
#ifdef ESP32
#include <SPIFFS.h>
#endif // ESP32
#include "FS.h"
#include "Datatypes/Interface.hpp"
#include "Datatypes/ConfigStruct.hpp"
#include "Utils/DeviceId.hpp"
#include "Utils/Validation.hpp"
#include "Constants.hpp"
#include "Limits.hpp"
#include "../SmartIotBootMode.hpp"
#include "../SmartIotNode.hpp"
#include "../SmartIotSetting.hpp"
#include "../StreamingOperator.hpp"

namespace SmartIotInternals {
class Config {
 public:
  Config();
  bool load();
  inline const ConfigStruct& get() const;
  char* getSafeConfigFile() const;
  void erase();
  void setSmartIotBootModeOnNextBoot(SmartIotBootMode bootMode);
  SmartIotBootMode getSmartIotBootModeOnNextBoot();
  void write(const JsonObject config);
  bool write(const char* config);
  
  bool patch(const char* patch);
  void log() const;  // print the current config to log output
  bool isValid() const;

 private:
  ConfigStruct _configStruct;
  bool _spiffsBegan;
  bool _valid;

  bool _spiffsBegin();
  void _patchJsonObject(JsonObject object, JsonObject patch);
};

const ConfigStruct& Config::get() const {
  return _configStruct;
}
}  // namespace SmartIotInternals
