#include "SmartIotNode.hpp"
#include "SmartIot.hpp"

using namespace SmartIotInternals;

std::vector<SmartIotNode*> SmartIotNode::nodes;

PropertyInterface::PropertyInterface()
: _property(nullptr) {
}

PropertyInterface& PropertyInterface::settable(const PropertyInputHandler& inputHandler) {
  _property->settable(inputHandler);
  return *this;
}

PropertyInterface& PropertyInterface::setName(const char* name) {
  _property->setName(name);
  return *this;
}

PropertyInterface& PropertyInterface::setUnit(const char* unit) {
  _property->setUnit(unit);
  return *this;
}

PropertyInterface& PropertyInterface::setDatatype(const char* datatype) {
  _property->setDatatype(datatype);
  return *this;
}

PropertyInterface& PropertyInterface::setFormat(const char* format) {
  _property->setFormat(format);
  return *this;
}

PropertyInterface& PropertyInterface::setRetained(const bool retained) {
  _property->setRetained(retained);
  return *this;
}

PropertyInterface& PropertyInterface::setProperty(Property* property) {
  _property = property;
  return *this;
}

SmartIotNode::SmartIotNode(const char* id, const char* name, const char* type, const NodeInputHandler& inputHandler)
: _id(id)
, _name(name)
, _type(type)
, runLoopDisconnected(false)
, _properties()
, _inputHandler(inputHandler) {
  if (strlen(id) + 1 > MAX_NODE_ID_LENGTH || strlen(type) + 1 > MAX_NODE_TYPE_LENGTH) {
    Helpers::abort(F("✖ SmartIotNode(): either the id or type string is too long"));
    return;  // never reached, here for clarity
  }
  SmartIot._checkBeforeSetup(F("SmartIotNode::SmartIotNode"));

  SmartIotNode::nodes.push_back(this);
}

SmartIotNode::~SmartIotNode() {
    Helpers::abort(F("✖✖ ~SmartIotNode(): Destruction of SmartIotNode object not possible\n  Hint: Don't create SmartIotNode objects as a local variable (e.g. in setup())"));
    return;  // never reached, here for clarity
}

uint16_t SmartIotNode::send(const String& value) {
  if (!Interface::get().ready) {
    Interface::get().getLogger() << F("✖ setNodeProperty(): impossible now") << endl;
    return 0;
  }

  char* topic = new char[strlen(Interface::get().getConfig().get().mqtt.baseTopic) + strlen(_name) + 1 + strlen(_type) + 1]; 
  
  strcpy(topic, Interface::get().getConfig().get().mqtt.baseTopic);
  strcat(topic, _type);
  strcat_P(topic, PSTR("/"));
  strcat(topic, _name);

  uint16_t packetId = Interface::get().getMqttClient().publish(topic, 1, true, value.c_str());

  delete[] topic;

  return packetId;
}

PropertyInterface& SmartIotNode::advertise(const char* id) {
  Property* propertyObject = new Property(id);

  _properties.push_back(propertyObject);

  return _propertyInterface.setProperty(propertyObject);
}

SendingPromise& SmartIotNode::setProperty(const String& property) const {
  Property* iProperty = this->getProperty(property);
  if (iProperty &&  iProperty->isRetained()) {
      return Interface::get().getSendingPromise().setNode(*this).setProperty(property).setQos(1).setRetained(true);
  } else {
      return Interface::get().getSendingPromise().setNode(*this).setProperty(property).setQos(1);
  }
}

Property* SmartIotNode::getProperty(const String& property) const {
  for (Property* iProperty : getProperties()) {
    if (strcmp(iProperty->getId(), property.c_str()) == 0)
       return iProperty;
  }
  return NULL;
}

bool SmartIotNode::handleInput(const String& value) {
  return _inputHandler(value);
}

const std::vector<SmartIotInternals::Property*>& SmartIotNode::getProperties() const {
  return _properties;
}
