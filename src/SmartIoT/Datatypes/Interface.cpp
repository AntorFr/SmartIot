#include "Interface.hpp"

using namespace SmartIotInternals;

InterfaceData Interface::_interface;  // need to define the static variable

InterfaceData::InterfaceData()
  : brand{ '\0' }
  , bootMode{ SmartIotBootMode::UNDEFINED }
  , configurationAp{ .secured = false, .password = {'\0'} }
  , firmware{ .name = {'\0'}, .version = {'\0'} }
  , led{ .enabled = false, .pin = 0, .on = 0 }
  , reset{ .enabled = false, .idle = false, .triggerPin = 0, .triggerState = 0, .triggerTime = 0, .resetFlag = false }
  , disable{ false }
  , flaggedForSleep{ false }
  , event{}
  , ready{ false }
  , _logger{ nullptr }
  , _blinker{ nullptr }
  , _time{ nullptr }
  , _config{ nullptr }
  , _mqttClient{ nullptr }
  , _loopFunction{ nullptr }
  , _uptime{ nullptr}
  , _sendingPromise{ nullptr } {
}

InterfaceData& Interface::get() {
  return _interface;
}
