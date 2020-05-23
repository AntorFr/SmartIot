#include "SmartIotSetting.hpp"

using namespace SmartIotInternals;

std::vector<ISmartIotSetting*> __attribute__((init_priority(101))) ISmartIotSetting::settings;

SmartIotInternals::ISmartIotSetting::ISmartIotSetting(const char * name, const char * description)
  : _name(name)
  , _description(description)
  , _required(true)
  , _provided(false) {
}

bool ISmartIotSetting::isRequired() const {
  return _required;
}

const char* ISmartIotSetting::getName() const {
  return _name;
}

const char* ISmartIotSetting::getDescription() const {
  return _description;
}



template <class T>
SmartIotSetting<T>::SmartIotSetting(const char* name, const char* description)
  : ISmartIotSetting(name, description)
  , _value()
  , _validator([](T candidate) { return true; }) {
  ISmartIotSetting::settings.push_back(this);
}

template <class T>
T SmartIotSetting<T>::get() const {
  return _value;
}

template <class T>
bool SmartIotSetting<T>::wasProvided() const {
  return _provided;
}

template <class T>
SmartIotSetting<T>& SmartIotSetting<T>::setDefaultValue(T defaultValue) {
  _value = defaultValue;
  _required = false;
  return *this;
}

template <class T>
SmartIotSetting<T>& SmartIotSetting<T>::setValidator(const std::function<bool(T candidate)>& validator) {
  _validator = validator;
  return *this;
}

template <class T>
bool SmartIotSetting<T>::validate(T candidate) const {
  return _validator(candidate);
}

template <class T>
void SmartIotSetting<T>::set(T value) {
  _value = value;
  _provided = true;
}

template <class T>
bool SmartIotSetting<T>::isBool() const { return false; }

template <class T>
bool SmartIotSetting<T>::isInt() const { return false; }

template <class T>
bool SmartIotSetting<T>::isLong() const { return false; }

template <class T>
bool SmartIotSetting<T>::isDouble() const { return false; }

template <class T>
bool SmartIotSetting<T>::isConstChar() const { return false; }

template<>
bool SmartIotSetting<bool>::isBool() const { return true; }
template<>
const char* SmartIotSetting<bool>::getType() const { return "bool"; }

template<>
bool SmartIotSetting<int32_t>::isInt() const { return true; }
template<>
const char* SmartIotSetting<int32_t>::getType() const { return "int"; }

template<>
bool SmartIotSetting<long>::isLong() const { return true; }
template<>
const char* SmartIotSetting<long>::getType() const { return "long"; }

template<>
bool SmartIotSetting<double>::isDouble() const { return true; }
template<>
const char* SmartIotSetting<double>::getType() const { return "double"; }

template<>
bool SmartIotSetting<const char*>::isConstChar() const { return true; }
template<>
const char* SmartIotSetting<const char*>::getType() const { return "string"; }

// Needed because otherwise undefined reference to
template class SmartIotSetting<bool>;
template class SmartIotSetting<int>;
template class SmartIotSetting<long>;
template class SmartIotSetting<double>;
template class SmartIotSetting<const char*>;
