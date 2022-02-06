#include <SmartIot.h>
#include <Nodes/SmartIotLedMatrix.h>

#define firmwareVersion "2.6.1"
#define firmwareName "ledMatrix"

SmartIotLedMatrix ledMatrixNode("led", "led", "led");

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  SmartIot_setFirmware(firmwareName, firmwareVersion);

  ledMatrixNode.setupMatrix<32, 8, VERTICAL_ZIGZAG_MATRIX>("matrix");

  SmartIot.setup();
  
  ledMatrixNode.addLeds<D1, GRB>(); //RGB Sapin //GRB / NEOPIXEL strip led
  //ledMatrixNode.setBrightness(10);
}

void loop() {
  SmartIot.loop();
}