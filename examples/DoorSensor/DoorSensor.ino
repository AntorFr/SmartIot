#include <SmartIot.h>

const int PIN_DOOR = 16;

Bounce debouncer = Bounce(); // Bounce is built into SmartIot, so you can use it without including it first
int lastDoorValue = -1;

SmartIotNode doorNode("door", "Door", "door");

void loopHandler() {
  int doorValue = debouncer.read();

  if (doorValue != lastDoorValue) {
     SmartIot.getLogger() << "Door is now " << (doorValue ? "open" : "close") << endl;

     doorNode.setProperty("open").send(doorValue ? "true" : "false");
     lastDoorValue = doorValue;
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  pinMode(PIN_DOOR, INPUT);
  digitalWrite(PIN_DOOR, HIGH);
  debouncer.attach(PIN_DOOR);
  debouncer.interval(50);

  SmartIot_setFirmware("awesome-door", "1.0.0");
  SmartIot.setLoopFunction(loopHandler);

  doorNode.advertise("open").setName("Open").setDatatype("boolean");

  SmartIot.setup();
}

void loop() {
  SmartIot.loop();
  debouncer.update();
}
