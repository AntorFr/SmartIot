#include <SmartIot.h>

#define firmwareVersion "1.0.0"
const int PIN_RELAY = 5;

SmartIotNode lightNode("light", "Light", "switch");

bool lightOnHandler(const SmartIotRange& range, const String& value) {
  if (value != "true" && value != "false") return false;

  bool on = (value == "true");
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  lightNode.setProperty("on").send(value);
  SmartIot.getLogger() << "Light is " << (on ? "on" : "off") << endl;

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  SmartIot_setFirmware("awesome-relay", firmwareVersion);

  lightNode.advertise("on").setName("On").setDatatype("boolean").settable(lightOnHandler);

  SmartIot.setup();
}

void loop() {
  SmartIot.loop();
}
