#ifdef ESP32

#include "FindDeskDevice.hpp"

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
using namespace SmartIotInternals;

FindDeskDeviceCallbacks::FindDeskDeviceCallbacks(std::string arg_address, SmartIotLinak *arg_desk) {
    this->ble_address_ = arg_address;
    this->desk_ = arg_desk;
  }

void FindDeskDeviceCallbacks::onResult(BLEAdvertisedDevice advertised_device) {
    Interface::get().getLogger() << F("BLE Device found: ") << advertised_device.toString().c_str() << endl;
    if (advertised_device.getAddress().equals(BLEAddress(this->ble_address_.c_str()))) {
        this->desk_->set_ble_device(new BLEAdvertisedDevice(advertised_device));
    }
}
#endif // ESP32