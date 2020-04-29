#include <SmartIot.h>

bool broadcastHandler(const String& level, const String& value) {
  SmartIot.getLogger() << "Received broadcast level " << level << ": " << value << endl;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  SmartIot_setFirmware("broadcast-test", "1.0.0");
  SmartIot.setBroadcastHandler(broadcastHandler);

  SmartIot.setup();
}

void loop() {
  SmartIot.loop();
}
