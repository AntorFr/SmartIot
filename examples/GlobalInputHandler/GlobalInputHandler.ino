#include <SmartIot.h>

SmartIotNode lightNode("light", "Light", "switch");

bool globalInputHandler(const SmartIotNode& node, const SmartIotRange& range, const String& property, const String& value) {
  SmartIot.getLogger() << "Received on node " << node.getId() << ": " << property << " = " << value << endl;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  SmartIot_setFirmware("global-input-handler", "1.0.0");
  SmartIot.setGlobalInputHandler(globalInputHandler);

  lightNode.advertise("on").setName("On").setDatatype("boolean").settable();

  SmartIot.setup();
}

void loop() {
  SmartIot.loop();
}
