#pragma once

#include "Arduino.h"
#include "StreamingOperator.hpp"
#include "SmartIot/Datatypes/Interface.hpp"
#include "SmartIotRange.hpp"

class SmartIotNode;

namespace SmartIotInternals {
class SendingPromise {
  friend ::SmartIotNode;

 public:
  SendingPromise();
  SendingPromise& setQos(uint8_t qos);
  SendingPromise& setRetained(bool retained);
  SendingPromise& overwriteSetter(bool overwrite);
  SendingPromise& setRange(const SmartIotRange& range);
  SendingPromise& setRange(uint16_t rangeIndex);
  uint16_t send(const String& value);

 private:
  SendingPromise& setNode(const SmartIotNode& node);
  SendingPromise& setProperty(const String& property);
  const SmartIotNode* getNode() const;
  const String* getProperty() const;
  uint8_t getQos() const;
  SmartIotRange getRange() const;
  bool isRetained() const;
  bool doesOverwriteSetter() const;

  const SmartIotNode* _node;
  const String* _property;
  uint8_t _qos;
  bool _retained;
  bool _overwriteSetter;
  SmartIotRange _range;
};
}  // namespace SmartIotInternals
