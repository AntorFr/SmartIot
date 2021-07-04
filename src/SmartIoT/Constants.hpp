#pragma once

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif // ESP32

#ifndef SMARTIOT_CONFIG
#define SMARTIOT_CONFIG 0 // config = 0 web server config desable
#endif

namespace SmartIotInternals {
  const char SMARTIOT_VERSION[] = "4.2.0";

  const IPAddress ACCESS_POINT_IP(192, 168, 3, 1);

  const uint16_t DEFAULT_MQTT_PORT = 1883;
  const char DEFAULT_MQTT_BASE_TOPIC[] = "home/";

  const uint8_t DEFAULT_RESET_PIN = 99; 
  const uint8_t DEFAULT_RESET_STATE = LOW;
  const uint16_t DEFAULT_RESET_TIME = 5 * 1000;

  const char DEFAULT_BRAND[] = "SmartIot";

  const uint16_t CONFIG_SCAN_INTERVAL = 20 * 1000;
  const uint32_t STATS_SEND_INTERVAL_SEC = 1 * 60;
  const uint16_t MQTT_RECONNECT_INITIAL_INTERVAL = 1000;
  const uint8_t MQTT_RECONNECT_MAX_BACKOFF = 6;

  const float LED_WIFI_DELAY = 1;
  const float LED_MQTT_DELAY = 0.2;

  const char CONFIG_UI_BUNDLE_PATH[] = "/SmartIoT/ui_bundle.gz";
  const char CONFIG_NEXT_BOOT_MODE_FILE_PATH[] = "/SmartIoT/NEXTMODE";
  const char SETUP_FILE_PATH[] = "/SmartIoT/setup.json";
}  // namespace SmartIotInternals
