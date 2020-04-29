#pragma once

#include "Arduino.h"

namespace SmartIotInternals {
class Uptime {
 public:
  Uptime();
  void update();
  uint64_t getSeconds() const;

 private:
  uint64_t _milliseconds;
  uint32_t _lastTick;
};
}  // namespace SmartIotInternals
