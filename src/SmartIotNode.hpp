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
class Config;
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
  PropertyInterface&  overwriteSetter(const bool overwrite = true);
  uint16_t send(const String& value);

 private:
  PropertyInterface& setProperty(Property* property);

  Property* _property;
};

class Property {
  friend SmartIotNode;
  friend BootNormal;

 public:
  explicit Property(const char* id, const SmartIotNode& node) {
    _id = strdup(id); _name = ""; _unit = ""; _datatype = ""; _format = ""; _retained = true; _settable = false; _node= &node; _overwriteSetter=false; }
  void settable(const PropertyInputHandler& inputHandler) { _settable = true;  _inputHandler = inputHandler; }
  void setName(const char* name) { _name = name; }
  void setUnit(const char* unit) { _unit = unit; }
  void setDatatype(const char* datatype) { _datatype = datatype; }
  void setFormat(const char* format) { _format = format; }
  void setRetained(const bool retained = true) { _retained = retained; }
  void overwriteSetter(const bool  overwrite = true) { _overwriteSetter = overwrite;}
  uint16_t send(const String& value);


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
  bool _overwriteSetter;
  const SmartIotNode* _node;
  PropertyInputHandler _inputHandler;
};
}  // namespace SmartIotInternals

class SmartIotNode {
  friend SmartIotInternals::SmartIotClass;
  friend SmartIotInternals::BootNormal;
  friend SmartIotInternals::BootConfig;
  friend SmartIotInternals::Config;

 public:
  SmartIotNode(const char* id, const char* name, const char* type, const SmartIotInternals::NodeInputHandler& nodeInputHandler = [](const String& value) { return false; });
  virtual ~SmartIotNode();

  const char* getId() const { return _id; }
  const char* getType() const { return _type; }
  const char* getName() const {return _name; }

  void setRetained(const bool retained = false) { _retained = retained; }
  void setName(const char* name ) { _name=name;}
  void setHandler(const SmartIotInternals::NodeInputHandler& inputHandler) {_settable = true; _inputHandler = inputHandler; } 
  bool isSettable() const { return _settable; }
  void notSettable() { _settable = false; }
  bool isRetained() const { return _retained; }
  void overwriteSetter(const bool  overwrite = true) { _overwriteSetter = overwrite;}
  bool doesOverwriteSetter() const { return _overwriteSetter; }

  uint16_t send(const JsonObject& data);
  uint16_t send(const String& value);

  SmartIotInternals::PropertyInterface& advertise(const char* id);
  SmartIotInternals::SendingPromise& setProperty(const String& property) const;
  SmartIotInternals::Property* getProperty(const String& property) const;

  void setRunLoopDisconnected(bool runLoopDisconnected) {
    this->runLoopDisconnected = runLoopDisconnected;
  }

 protected:
  virtual void setup();
  virtual void loop() {}
  virtual void onReadyToOperate() {}
  virtual void stop() {}
  virtual bool handleInput(const String& value);
  virtual bool loadNodeConfig(ArduinoJson::JsonObject& data);
  virtual void publish_stats() {}

  static std::vector<SmartIotNode*> nodes;

  static SmartIotNode* find(const char* id) {
    for (SmartIotNode* iNode : SmartIotNode::nodes) {
      if (strcmp(id, iNode->getId()) == 0) return iNode;
    }
    for (SmartIotNode* iNode : SmartIotNode::nodes) {
      if (strcmp(id, iNode->getName()) == 0) return iNode;
    }
    return 0;
  }

    static SmartIotNode* find(const char* name, const char* type) {
    for (SmartIotNode* iNode : SmartIotNode::nodes) {
      if ((strcmp(name, iNode->getName()) == 0) && (strcmp(type, iNode->getType()) == 0)) return iNode;
    }
    return 0;
  }

 private:
  const std::vector<SmartIotInternals::Property*>& getProperties() const;
  const char* _id;
  const char* _name;
  const char* _type;
  bool _settable;
  bool _retained;
  bool _overwriteSetter;

  //to be remove >
  bool _range;
  uint16_t _lower;
  uint16_t _upper;
  //<

  bool runLoopDisconnected;

  std::unique_ptr<char[]> _mqttTopic;


  std::vector<SmartIotInternals::Property*> _properties;
  SmartIotInternals::NodeInputHandler _inputHandler;

  SmartIotInternals::PropertyInterface _propertyInterface;


};
