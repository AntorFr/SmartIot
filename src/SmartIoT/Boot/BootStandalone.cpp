#include "BootStandalone.hpp"

using namespace SmartIotInternals;

BootStandalone::BootStandalone()
  : Boot("standalone") {
}

BootStandalone::~BootStandalone() {
}

void BootStandalone::setup() {
  Boot::setup();

  WiFi.mode(WIFI_OFF);

#if SMARTIOT_CONFIG
  ResetHandler::Attach();
#endif
}

void BootStandalone::loop() {
  Boot::loop();
}
