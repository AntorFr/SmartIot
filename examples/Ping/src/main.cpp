#include <Arduino.h>
#include "PingNode.hpp"

PingNode pingNode("ping", "Ping");

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  SmartIot_setFirmware("ping-demo", "1.0.0"); // The underscore is not a typo! See Magic bytes

  SmartIot.setup();
  SmartIot.getLogger() << F("✔ main: Setup ready") << endl;
  SmartIot.getLogger() << F("  send now ping request via MQTT to /SmartIoT/") << SmartIot.getConfiguration().deviceId << F("/ping/ping/set") << endl;
}

void loop() {
  SmartIot.loop();
}
