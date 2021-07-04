#include <SmartIot.h>

const int PIN_BUTTON = 5; // D1

int buttonState = 0;
bool buttonPressed = false;

SmartIotNode buttonNode("button", "Button", "button");

void loopHandler() {
  buttonState = !digitalRead(PIN_BUTTON);

  if (buttonState == HIGH && !buttonPressed) {
    buttonNode.setProperty("button").send("PRESSED");
    SmartIot.getLogger() << "Button pressed" << endl;
    buttonPressed = true;
  } else if (buttonState == LOW && buttonPressed) {
    buttonNode.setProperty("button").send("RELEASED");
    SmartIot.getLogger() << "Button released" << endl;
    buttonPressed = false;
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  SmartIot_setFirmware("awesome-button", "1.0.0");
  SmartIot.setLoopFunction(loopHandler);

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  buttonNode.advertise("button").setName("Button")
                                .setDatatype("enum")
                                .setFormat("PRESSED,RELEASED")
                                .setRetained(false);

  SmartIot.setup();
}

void loop() {
  SmartIot.loop();
}
