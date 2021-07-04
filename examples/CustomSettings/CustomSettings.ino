#include <SmartIot.h>

const int DEFAULT_TEMPERATURE_INTERVAL = 300;

unsigned long lastTemperatureSent = 0;

SmartIotNode temperatureNode("temperature", "Temperature", "temperature");

SmartIotSetting<long> temperatureIntervalSetting("temperatureInterval", "The temperature interval in seconds");

void loopHandler() {
  if (millis() - lastTemperatureSent >= temperatureIntervalSetting.get() * 1000UL || lastTemperatureSent == 0) {
    float temperature = 22; // Fake temperature here, for the example
    SmartIot.getLogger() << "Temperature: " << temperature << " °C" << endl;
    temperatureNode.setProperty("degrees").send(String(temperature));
    lastTemperatureSent = millis();
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  SmartIot_setFirmware("temperature-setting", "1.0.0");
  SmartIot.setLoopFunction(loopHandler);

  temperatureNode.advertise("degrees").setName("Degrees")
                                      .setDatatype("float")
                                      .setUnit("ºC");

  temperatureIntervalSetting.setDefaultValue(DEFAULT_TEMPERATURE_INTERVAL).setValidator([] (long candidate) {
    return candidate > 0;
  });

  SmartIot.setup();
}

void loop() {
  SmartIot.loop();
}
