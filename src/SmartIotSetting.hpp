#pragma once

#include <vector>
#include <functional>
#include "Arduino.h"

#include "./SmartIot/Datatypes/Callbacks.hpp"

namespace SmartIotInternals {
class SmartIotClass;
class Config;
class Validation;
class BootConfig;

class ISmartIotSetting {
 public:
  static std::vector<ISmartIotSetting*> settings;

  bool isRequired() const;
  const char* getName() const;
  const char* getDescription() const;

  virtual bool isBool() const { return false; }
  virtual bool isInt() const { return false; }
  virtual bool isLong() const { return false; }
  virtual bool isDouble() const { return false; }
  virtual bool isConstChar() const { return false; }

  virtual const char* getType() const { return "unknown"; }

 protected:
  explicit ISmartIotSetting(const char* name, const char* description);
  const char* _name;
  const char* _description;
  bool _required;
  bool _provided;
};
}  // namespace SmartIotInternals

template <class T>
class SmartIotSetting : public SmartIotInternals::ISmartIotSetting {
  friend SmartIotInternals::SmartIotClass;
  friend SmartIotInternals::Config;
  friend SmartIotInternals::Validation;
  friend SmartIotInternals::BootConfig;

 public:
  SmartIotSetting(const char* name, const char* description);
  T get() const;
  bool wasProvided() const;
  SmartIotSetting<T>& setDefaultValue(T defaultValue);
  SmartIotSetting<T>& setValidator(const std::function<bool(T candidate)>& validator);

 private:
  T _value;
  std::function<bool(T candidate)> _validator;

  bool validate(T candidate) const;
  void set(T value);

  bool isBool() const;
  bool isInt() const;
  bool isLong() const;
  bool isDouble() const;
  bool isConstChar() const;

  const char* getType() const;
};
