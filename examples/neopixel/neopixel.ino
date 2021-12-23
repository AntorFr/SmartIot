#include <SmartIot.h>
#include <Nodes/SmartIotLed.h>

#define firmwareVersion "2.2.1"
#define firmwareName "led"

SmartIotLed ledNode("led", "led", "led");

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  SmartIot_setFirmware(firmwareName, firmwareVersion);

  SmartIot.setup();
   
  ledNode.addLeds<D1,GRB>(); //RGB Sapin //GRB / NEOPIXEL strip led

  
}

void loop() {
  SmartIot.loop();
}
