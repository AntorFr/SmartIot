#pragma once

#include "Arduino.h"

#include "Boot.hpp"
#include "../../StreamingOperator.hpp"
#include "../Utils/ResetHandler.hpp"

namespace SmartIotInternals {
class BootStandalone : public Boot {
 public:
  BootStandalone();
  ~BootStandalone();
  void setup();
  void loop();
};
}  // namespace SmartIotInternals
