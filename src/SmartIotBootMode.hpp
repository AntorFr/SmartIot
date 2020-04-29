#pragma once

enum class SmartIotBootMode : uint8_t {
  UNDEFINED = 0,
  STANDALONE = 1,
#if SMARTIOT_CONFIG
  CONFIGURATION = 2,
#endif
  NORMAL = 3
};
