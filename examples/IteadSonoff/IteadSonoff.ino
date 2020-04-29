/* WARNING: untested */

#include <SmartIot.h>

const int PIN_RELAY = 12;
const int PIN_LED = 13;
const int PIN_BUTTON = 0;

SmartIotNode switchNode("switch", "Switch", "switch");

bool switchOnHandler(const SmartIotRange& range, const String& value) {
  if (value != "true" && value != "false") return false;

  bool on = (value == "true");
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  switchNode.setProperty("on").send(value);
  SmartIot.getLogger() << "Switch is " << (on ? "on" : "off") << endl;

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  SmartIot_setFirmware("itead-sonoff", "1.0.0");
  SmartIot.setLedPin(PIN_LED, LOW).setResetTrigger(PIN_BUTTON, LOW, 5000);

  switchNode.advertise("on").setName("On").setDatatype("boolean").settable(switchOnHandler);

  SmartIot.setup();
}

void loop() {
  SmartIot.loop();
}
