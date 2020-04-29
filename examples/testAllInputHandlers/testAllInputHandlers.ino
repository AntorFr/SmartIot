/*

This is a sketch testing all the different types of handlers
The global input handler will always be triggered.
Setting lightnode1/property1/ will trigger the node input handler.
Setting lightnode2/property1/ will trigger the property input handler.

*/

#include <Arduino.h>
#include <SmartIot.h>

bool globalInputHandler(const SmartIotNode& node, const SmartIotRange& range, const String& property, const String& value) {
  SmartIot.getLogger() << "Global input handler. Received on node " << node.getId() << ": " << property << " = " << value << endl;
  return false;
}

bool nodeInputHandler(const SmartIotRange & range, const String & property, const String & value) {
  SmartIot.getLogger() << "Node input handler. Received on property " << property << " value: " << value;
  return true;
}

bool propertyInputHandler(const SmartIotRange& range, const String& value) {
  SmartIot.getLogger() << "Property input handler. Receveived value: " << value;
  return true;
}

SmartIotNode lightNode1("lightnode1", "Light Node One Name","switch", false, 0 , 0, &nodeInputHandler);
SmartIotNode lightNode2("lightnode2", "Light Two One Name","switch");

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  SmartIot_setFirmware("Test all input handlers", "0.0.1");
  lightNode1.advertise("property1").setName("ln1 First property").setDatatype("boolean").settable();
  lightNode2.advertise("property1").setName("ln2 First property").setDatatype("boolean").settable(propertyInputHandler);
  SmartIot.setGlobalInputHandler(globalInputHandler);
  SmartIot.setup();
}

void loop() {
  SmartIot.loop();
}
