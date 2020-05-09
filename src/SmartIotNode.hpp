#pragma once

#include <functional>
#include <vector>
#include "Arduino.h"
#include "StreamingOperator.hpp"
#include "SmartIot/Datatypes/Interface.hpp"
#include "SmartIot/Datatypes/Callbacks.hpp"
#include "SmartIot/Limits.hpp"
#include "SmartIotRange.hpp"

class SmartIotNode;

namespace SmartIotInternals {
class SmartIotClass;
class Property;
class BootNormal;
class BootConfig;
class SendingPromise;

class PropertyInterface {
  friend ::SmartIotNode;

 public:
  PropertyInterface();

  PropertyInterface& settable(const PropertyInputHandler& inputHandler = [](const SmartIotRange& range, const String& value) { return false; });
  PropertyInterface& setName(const char* name);
  PropertyInterface& setUnit(const char* unit);
  PropertyInterface& setDatatype(const char* datatype);
  PropertyInterface& setFormat(const char* format);
  PropertyInterface& setRetained(const bool retained = true);

 private:
  PropertyInterface& setProperty(Property* property);

  Property* _property;
};

class Property {
  friend SmartIotNode;
  friend BootNormal;

 public:
  explicit Property(const char* id) {
    _id = strdup(id); _name = ""; _unit = ""; _datatype = ""; _format = ""; _retained = true; _settable = false; }
  void settable(const PropertyInputHandler& inputHandler) { _settable = true;  _inputHandler = inputHandler; }
  void setName(const char* name) { _name = name; }
  void setUnit(const char* unit) { _unit = unit; }
  void setDatatype(const char* datatype) { _datatype = datatype; }
  void setFormat(const char* format) { _format = format; }
  void setRetained(const bool retained = true) { _retained = retained; }


 private:
  const char* getId() const { return _id; }
  const char* getName() const { return _name; }
  const char* getUnit() const { return _unit; }
  const char* getDatatype() const { return _datatype; }
  const char* getFormat() const { return _format; }
  bool isRetained() const { return _retained; }
  bool isSettable() const { return _settable; }
  PropertyInputHandler getInputHandler() const { return _inputHandler; }
  const char* _id;
  const char* _name;
  const char* _unit;
  const char* _datatype;
  const char* _format;
  bool _retained;
  bool _settable;
  PropertyInputHandler _inputHandler;
};
}  // namespace SmartIotInternals

class SmartIotNode {
  friend SmartIotInternals::SmartIotClass;
  friend SmartIotInternals::BootNormal;
  friend SmartIotInternals::BootConfig;

 public:
  SmartIotNode(const char* id, const char* name, const char* type, bool range = false, uint16_t lower = 0, uint16_t upper = 0, const SmartIotInternals::NodeInputHandler& nodeInputHandler = [](const String& value) { return false; });
  virtual ~SmartIotNode();

  const char* getId() const { return _id; }
  const char* getType() const { return _type; }
  const char* getName() const {return _name; }
  bool isRange() const { return _range; }
  uint16_t getLower() const { return _lower; }
  uint16_t getUpper() const { return _upper; }

  
  uint16_t send(const String& value);

  SmartIotInternals::PropertyInterface& advertise(const char* id);
  SmartIotInternals::SendingPromise& setProperty(const String& property) const;
  SmartIotInternals::Property* getProperty(const String& property) const;

  void setRunLoopDisconnected(bool runLoopDisconnected) {
    this->runLoopDisconnected = runLoopDisconnected;
  }

 protected:
  virtual void setup() {}
  virtual void loop() {}
  virtual void onReadyToOperate() {}
  virtual bool handleInput(const String& value);

 private:
  const std::vector<SmartIotInternals::Property*>& getProperties() const;

  static SmartIotNode* find(const char* id) {
    for (SmartIotNode* iNode : SmartIotNode::nodes) {
      if (strcmp(id, iNode->getId()) == 0) return iNode;
    }

    return 0;
  }


  const char* _id;
  const char* _name;
  const char* _type;
  bool _range;
  uint16_t _lower;
  uint16_t _upper;

  bool runLoopDisconnected;

  std::vector<SmartIotInternals::Property*> _properties;
  SmartIotInternals::NodeInputHandler _inputHandler;

  SmartIotInternals::PropertyInterface _propertyInterface;

  static std::vector<SmartIotNode*> nodes;
};
