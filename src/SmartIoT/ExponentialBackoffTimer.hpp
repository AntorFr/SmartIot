#pragma once

#include "Timer.hpp"
#include "Datatypes/Interface.hpp"

namespace SmartIotInternals {
class ExponentialBackoffTimer {
 public:
  ExponentialBackoffTimer(uint16_t initialInterval, uint8_t maxBackoff);
  void activate();
  bool check();
  bool reachMax();
  void deactivate();
  bool isActive() const;

 private:
  Timer _timer;

  uint16_t _initialInterval;
  uint8_t _maxBackoff;
  uint8_t _retryCount;
};
}  // namespace SmartIotInternals
