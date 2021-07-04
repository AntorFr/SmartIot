#ifdef ESP32

#include "BLEDevice.h"
#include "SmartIotLinak.hpp"
#include <string>

#include <SmartIot.h>

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class FindDeskDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
 private:
  std::string ble_address_;
  SmartIotLinak *desk_;

 public:
  FindDeskDeviceCallbacks(std::string arg_address, SmartIotLinak *arg_desk);
  void onResult(BLEAdvertisedDevice advertised_device);
};

#endif // ESP32