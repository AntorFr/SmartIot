#pragma once

#include <ArduinoJson.h>

namespace SmartIotInternals {
  const uint16_t MAX_JSON_CONFIG_FILE_SIZE = 1500;

  const uint16_t JSON_MSG_BUFFER = 1024;

  // max setting elements
  const uint8_t MAX_CONFIG_SETTING_SIZE = 10;
  const uint8_t MAX_CONFIG_SETTING_NUMBER = 5;
  const uint8_t MAX_NODES_SETTING_SIZE = 15;
  const uint8_t MAX_NODES_SETTING_NUMBER = 10;
  // 6 elements at root, 9 elements at wifi, 6 elements at mqtt, 1 element at ota, max settings elements
  const uint16_t MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE = JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(1) + MAX_NODES_SETTING_NUMBER * JSON_OBJECT_SIZE(MAX_NODES_SETTING_SIZE) + MAX_CONFIG_SETTING_NUMBER * JSON_OBJECT_SIZE(MAX_CONFIG_SETTING_SIZE);

  const uint8_t MAX_MAC_LENGTH = 6;
  const uint8_t MAX_MAC_STRING_LENGTH = (MAX_MAC_LENGTH * 2) + 1;

  const uint8_t MAX_CHIPID_LENGTH = 8 + 1;

  const uint8_t MAX_WIFI_SSID_LENGTH = 32 + 1;
  const uint8_t MAX_WIFI_PASSWORD_LENGTH = 64 + 1;
  const uint16_t MAX_HOSTNAME_LENGTH = 255 + 1;
  const uint8_t MAX_FINGERPRINT_SIZE = 20;
  const uint8_t MAX_FINGERPRINT_STRING_LENGTH = (MAX_FINGERPRINT_SIZE *2) + 1;

  const uint8_t MAX_MQTT_CREDS_LENGTH = 32 + 1;
  const uint8_t MAX_MQTT_BASE_TOPIC_LENGTH = 48 + 1;
  const uint8_t MAX_MQTT_TOPIC_LENGTH = 128 + 1;

  const uint8_t MAX_FRIENDLY_NAME_LENGTH = 64 + 1;
  const uint8_t MAX_DEVICE_ID_LENGTH = 32 + 1;

  const uint8_t MAX_BRAND_LENGTH = MAX_WIFI_SSID_LENGTH - 10 - 1;
  const uint8_t MAX_FIRMWARE_NAME_LENGTH = 32 + 1;
  const uint8_t MAX_FIRMWARE_VERSION_LENGTH = 16 + 1;

  const uint8_t MAX_NODE_ID_LENGTH = 24 + 1;
  const uint8_t MAX_NODE_TYPE_LENGTH = 24 + 1;
  const uint8_t MAX_NODE_PROPERTY_LENGTH = 24 + 1;

  const uint8_t MAX_IP_STRING_LENGTH = 16 + 1;

}  // namespace SmartIotInternals
