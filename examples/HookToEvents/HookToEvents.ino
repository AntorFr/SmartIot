#include <SmartIot.h>

void onSmartIotEvent(const SmartIotEvent& event) {
  switch (event.type) {
    case SmartIotEventType::STANDALONE_MODE:
      Serial << "Standalone mode started" << endl;
      break;
    case SmartIotEventType::CONFIGURATION_MODE:
      Serial << "Configuration mode started" << endl;
      break;
    case SmartIotEventType::NORMAL_MODE:
      Serial << "Normal mode started" << endl;
      break;
    case SmartIotEventType::OTA_STARTED:
      Serial << "OTA started" << endl;
      break;
    case SmartIotEventType::OTA_PROGRESS:
      Serial << "OTA progress, " << event.sizeDone << "/" << event.sizeTotal << endl;
      break;
    case SmartIotEventType::OTA_FAILED:
      Serial << "OTA failed" << endl;
      break;
    case SmartIotEventType::OTA_SUCCESSFUL:
      Serial << "OTA successful" << endl;
      break;
    case SmartIotEventType::ABOUT_TO_RESET:
      Serial << "About to reset" << endl;
      break;
    case SmartIotEventType::WIFI_CONNECTED:
      Serial << "Wi-Fi connected, IP: " << event.ip << ", gateway: " << event.gateway << ", mask: " << event.mask << endl;
      break;
    case SmartIotEventType::WIFI_DISCONNECTED:
      Serial << "Wi-Fi disconnected, reason: " << (int8_t)event.wifiReason << endl;
      break;
    case SmartIotEventType::MQTT_READY:
      Serial << "MQTT connected" << endl;
      break;
    case SmartIotEventType::MQTT_DISCONNECTED:
      Serial << "MQTT disconnected, reason: " << (int8_t)event.mqttReason << endl;
      break;
    case SmartIotEventType::MQTT_PACKET_ACKNOWLEDGED:
      Serial << "MQTT packet acknowledged, packetId: " << event.packetId << endl;
      break;
    case SmartIotEventType::READY_TO_SLEEP:
      Serial << "Ready to sleep" << endl;
      break;
    case SmartIotEventType::SENDING_STATISTICS:
      Serial << "Sending statistics" << endl;
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  SmartIot.disableLogging();
  SmartIot_setFirmware("events-test", "1.0.0");
  SmartIot.onEvent(onSmartIotEvent);
  SmartIot.setup();
}

void loop() {
  SmartIot.loop();
}
