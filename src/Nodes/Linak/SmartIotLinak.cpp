
#ifdef ESP32
#include "SmartIotLinak.hpp"

#include <string>

using namespace SmartIotInternals;


static BLEUUID outputServiceUUID("99fa0020-338a-1024-8a49-009c0215f78a");
static BLEUUID outputCharacteristicUUID("99fa0021-338a-1024-8a49-009c0215f78a");
static BLEUUID inputServiceUUID("99fa0030-338a-1024-8a49-009c0215f78a");
static BLEUUID inputCharacteristicUUID("99fa0031-338a-1024-8a49-009c0215f78a");
static BLEUUID controlServiceUUID("99fa0001-338a-1024-8a49-009c0215f78a");
static BLEUUID controlCharacteristicUUID("99fa0002-338a-1024-8a49-009c0215f78a");

static float deskMaxHeight = 65.0;


SmartIotLinak::SmartIotLinak(const char* id, const char* name, const char* type, const NodeInputHandler& inputHandler)
    :SmartIotNode(id,name,type,inputHandler) {
    setHandler([=](const String& json){ return this->SmartIotLinak::RequestHandler(json);});
}

void SmartIotLinak::setup() {
    SmartIotNode::setup();

    Interface::get().getLogger() << F("• Setting up Linak Desk Controller...") << endl;
    SmartIotLinak::instance = this;
    this->set_connection_(false);

    BLEDevice::init(this->getName());
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new FindDeskDeviceCallbacks(this->ble_address_, this));
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);

    this->p_client_ = BLEDevice::createClient();
    this->p_client_->setClientCallbacks(this);

    this->set_interval("update_desk", 200, [this]() { this->update_desk_(); });

    // Delay bluetooth scanning from a second
    this->set_timeout(1000, [this]() { this->scan_(); });

}

void SmartIotLinak::onReadyToOperate() {
    Interface::get().getLogger() << F("• Ready to operate Linak node ") << getName() << endl;
}

void SmartIotLinak::loop() {}

bool SmartIotLinak::loadNodeConfig(ArduinoJson::JsonObject& data){
    SmartIotNode::loadNodeConfig(data);

    ConfigValidationResult configValidationResult = Validation::validateConfig(data);
    if (!configValidationResult.valid) {
        Interface::get().getLogger() << F("✖ Linak config file is not valid, reason: ") << configValidationResult.reason << endl;
        return false;
    }

    if (data.containsKey("desk_address")) {
        strcpy(_deskBtAddress, data["desk_address"]);
    }
    return true;
}

bool SmartIotLinak::RequestHandler(const String& json){
    DynamicJsonDocument parseJsonBuff (5+ JSON_OBJECT_SIZE(1)); 
    DeserializationError error = deserializeJson(parseJsonBuff, json);
    if (error) {
        Interface::get().getLogger() << F("✖ Invalid JSON Switch commande: ") << error.c_str() << endl;
        return false;
    }
    JsonObject data = parseJsonBuff.as<JsonObject>();
    //serializeJsonPretty(data, Serial); 

    if(data.containsKey("move") {
        JsonVariant move = data["move"]
        if(move.containsKey("height") && move["height"].is<unsigned int>()){ 
            #ifdef DEBUG
                Interface::get().getLogger() << F("Linak node, handle value: ") << move["height"].as<uint16_t>() << endl;
            #endif // DEBUG
            return moveToHeightMm(move["height"].as<uint16_t>());
        }
        if(move.containsKey("position") && move["position"].is<unsigned int>()){ 

            #ifdef DEBUG
                Interface::get().getLogger() << F("Linak node, handle value: ") << move["height"].as<uint16_t>() << endl;
            #endif // DEBUG
            return moveToHeightMm(move["height"].as<uint16_t>());
        }
    }
    if(data.containsKey("save")) {
        JsonVariant save = data["save"]
        if(save.containsKey("position") && save["position"].is<unsigned int>()){ 
            return saveCurrentHeightAsFav(save["position"].as<uint8_t>())
        }
    } 
    return false;
}

void SmartIotLinak::publish_stats(){
    _connected = _controller.isConnected();
    bool moving = _ticker.active();
    uint16_t oldHeight = _height;
    bool heightChanged = false;

    if (_connected) {
        _height = _controller.getHeightMm();
        heightChanged = (_height != oldHeight);
    } 

    if ((!_connected) || (moving && !heightChanged) || (_height == _targetHeight)) {
            _ticker.detach();
    }

    if (!moving || (moving && heightChanged)) {
        DynamicJsonDocument jsonBuffer (JSON_OBJECT_SIZE(5)); 
        JsonObject data = jsonBuffer.to<JsonObject>();

        data["connected"] = _connected;
        data["height"] = _height;
        data["target_height"] = _targetHeight;
        data["moving"] = moving;
        //TODO

        send(data);
    }

}

ConfigValidationResult SmartIotLinak::_validateConfig(ArduinoJson::JsonObject& object) {
    ConfigValidationResult result;
    result.valid = false;

    JsonVariant deskAddress = object["desk_address"]; 

    if ((deskAddress.isNull()) {
        result.reason = F("desk_address is required");
        return result;
    }
    if (!deskAddress.as<const char*>()) {
        result.reason = F("desk_address is not a string");
        return result;
    }
    if (!Helpers::validateMacAddress(deskAddress.as<const char*>())) {
        result.reason = F("deskAddress is not valid mac");
        return result;
    }

}


static float transform_height_to_position(float height) { return height / deskMaxHeight; }
static float transform_position_to_height(float position) { return position * deskMaxHeight; }


static void writeUInt16(BLERemoteCharacteristic *charcteristic, unsigned short value) {
  uint8_t data[2];
  data[0] = value;
  data[1] = value >> 8;
  charcteristic->writeValue(data, 2);
}

static void deskHeightUpdateNotificationCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData,
                                                 size_t length, bool isNotify) {
  SmartIotLinak::instance->update_desk_data(pData);
};



SmartIotLinak *SmartIotLinak::instance;

void SmartIotLinak::set_mac_address(uint64_t address) {
  char buffer[24];
  auto *mac = reinterpret_cast<uint8_t *>(&address);
  snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  this->ble_address_ = std::string(buffer);
}

void SmartIotLinak::set_ble_device(BLEAdvertisedDevice *device) {
  BLEDevice::getScan()->stop();
  this->ble_device_ = device;

  this->set_timeout(200, [this]() { this->connect(); });
}

void SmartIotLinak::dump_config() {
  ESP_LOGCONFIG(TAG, "Idasen Desk Controller:");
  ESP_LOGCONFIG(TAG, "  Mac address: %s", this->ble_address_.c_str());
  ESP_LOGCONFIG(TAG, "  Bluetooth callback: %s", this->bluetooth_callback_ ? "true" : "false");
  LOG_SENSOR("  ", "Desk height", this->desk_height_sensor_);
  LOG_BINARY_SENSOR("  ", "Desk moving", this->desk_moving_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Desk connection", this->desk_connection_binary_sensor_);
  LOG_COVER("  ", "Desk", this);
}

void SmartIotLinak::scan_() {
  if (this->ble_device_ != nullptr) {
    return;
  }
  Interface::get().getLogger() << F("Start scanning devices...") << endl;
  BLEDevice::getScan()->start(5, false);
  this->set_timeout(10000, [this]() { this->scan_(); });
}

void SmartIotLinak::connect() {
    if (this->p_client_->isConnected()) {
        return;
    }

    Interface::get().getLogger() << F("Connecting client to device ") << this->ble_device_->getAddress().toString().c_str()  << endl;
    this->p_client_->connect(this->ble_device_);

    if (false == this->p_client_->isConnected()) {
        Interface::get().getLogger() << F("Fail to connect to client") << endl;
        this->set_timeout(5000, [this]() { this->connect(); });
        return;
     }

  if (this->m_input_char_ == nullptr) {
        Interface::get().getLogger() << F("Retrieve input remote characteristic.") << endl;
        this->m_input_char_ = this->p_client_->getService(inputServiceUUID)->getCharacteristic(inputCharacteristicUUID);
  }

  if (this->m_output_char_ == nullptr) {
        Interface::get().getLogger() << F("Retrieve output remote characteristic.") << endl;
        this->m_output_char_ = this->p_client_->getService(outputServiceUUID)->getCharacteristic(outputCharacteristicUUID);

    // Register bluetooth callback
    if (this->bluetooth_callback_ && this->m_output_char_->canNotify()) {
        Interface::get().getLogger() << F("Register notification callback on output characteristic.") << endl;
        this->m_output_char_->registerForNotify(deskHeightUpdateNotificationCallback, true);
    }
  }

  if (this->m_control_char_ == nullptr) {
        Interface::get().getLogger() << F("Retrieve control remote characteristic.") << endl;
        this->m_control_char_ =
            this->p_client_->getService(controlServiceUUID)->getCharacteristic(controlCharacteristicUUID);
  }

  Interface::get().getLogger() << F("Success connecting client to device.") << endl;

  // Delay data sync from 5s
  this->set_timeout(5000, [this]() { this->update_desk_data(); });
}

void SmartIotLinak::onConnect(BLEClient *p_client) { this->set_connection_(true); }

void SmartIotLinak::onDisconnect(BLEClient *p_client) {
  this->set_connection_(false);
  this->set_timeout(2000, [this]() { this->connect(); });
}

void SmartIotLinak::set_connection_(bool connected) {
  if (this->desk_connection_binary_sensor_) {
    this->desk_connection_binary_sensor_->publish_state(connected);
  }
}

void SmartIotLinak::update_desk_data(uint8_t *pData, bool allow_publishing_cover_state) {
  float height;
  float speed;
  bool callback_data = pData != nullptr;

  unsigned short height_mm;
  if (callback_data) {
    // Data from the callback
    height_mm = (*(uint16_t *) pData) / 10;
    speed = (*(uint16_t *) (pData + 2)) / 100;
  } else {
    std::string value = this->m_output_char_->readValue();

    height_mm = (*(uint16_t *) (value.data())) / 10;
    speed = (*(uint16_t *) (value.data() + 2)) / 100;
  }

  height = (float) height_mm / 10;

  Interface::get().getLogger() << F("Desk bluetooth data: height: ") << height << F(" speed: ") << speed << endl;

  // set height sensor
  if (this->desk_height_sensor_ != nullptr) {
    if (this->get_heigth_() != height) {
      this->desk_height_sensor_->publish_state(height);
    }
  }

  // set moving state
  bool moving = speed > 0;
  bool moving_updated = false;
  if (this->desk_moving_binary_sensor_ != nullptr) {
    if (!this->desk_moving_binary_sensor_->has_state() || this->desk_moving_binary_sensor_->state != moving) {
      this->desk_moving_binary_sensor_->publish_state(moving);
      moving_updated = true;
    }
  }

  if (!allow_publishing_cover_state) {
    return;
  }

  // Publish cover state
  float position = transform_height_to_position(height);
  bool position_updated = position != this->position;
  bool operation_updated = this->current_operation_ != this->current_operation;

  // No updated needed when nothing has changed
  if (!position_updated && !operation_updated) {
    return;
  }

  // Only position has been updated when moving
  if (!moving_updated && !operation_updated && moving) {
    return;
  }

  this->publish_cover_state_(position);
}

void SmartIotLinak::publish_cover_state_(float position) {
  this->position = position;
  this->current_operation = this->current_operation_;
  this->set_timeout("update_cover_state", 1, [this]() { this->publish_state(false); });
}

float SmartIotLinak::get_heigth_() {
  if (this->desk_height_sensor_ == nullptr || !this->desk_height_sensor_->has_state()) {
    return 0;
  }

  return this->desk_height_sensor_->get_raw_state();
}

void SmartIotLinak::update_desk_() {
  // Was stopped
  if (this->current_operation_ == cover::COVER_OPERATION_IDLE) {
    return;
  }

  if (!this->bluetooth_callback_) {
    this->update_desk_data(nullptr, false);
  }

  // Retrieve current desk height
  float height = this->get_heigth_();

  // Check if target has been reached
  if (this->is_at_target_(height)) {
    Interface::get().getLogger() << F("Update Desk - target reached") << endl;
    this->stop_move_();
    return;
  }
  Interface::get().getLogger() << F("Update Desk - Move from ") << height << F(" to ") << this->height_target_ << endl;
  this->move_torwards_();
}

void SmartIotLinak::control(const cover::CoverCall &call) {
  if (call.get_position().has_value()) {
    if (this->current_operation_ != cover::COVER_OPERATION_IDLE) {
      this->stop_move_();
    }

    float position_target = *call.get_position();
    this->height_target_ = transform_position_to_height(position_target);
    this->update_desk_data(nullptr, false);
    float height = this->get_heigth_();
    ESP_LOGD(TAG, "Cover control - START - position %.1f - target %.1f - current %.1f", position_target,
             this->height_target_, height);

    if (this->height_target_ == height) {
      return;
    }

    if (this->height_target_ > height) {
      this->current_operation_ = cover::COVER_OPERATION_OPENING;
    } else {
      this->current_operation_ = cover::COVER_OPERATION_CLOSING;
    }

    // Prevent from potential stop moving update @see SmartIotLinak::stop_move_()
    this->cancel_timeout("stop_moving_update");

    // Instead publish cover data
    this->publish_cover_state_(transform_height_to_position(height));

    this->start_move_torwards_();
    return;
  }

  if (call.get_stop()) {
    ESP_LOGD(TAG, "Cover control - STOP");
    this->stop_move_();
  }
}

void SmartIotLinak::start_move_torwards_() {
  writeUInt16(this->m_control_char_, 0xFE);
  writeUInt16(this->m_control_char_, 0xFF);
}

void SmartIotLinak::move_torwards_() {
  writeUInt16(this->m_input_char_, (unsigned short) (this->height_target_ * 100));
}

void SmartIotLinak::stop_move_() {
  writeUInt16(this->m_control_char_, 0xFF);
  writeUInt16(this->m_input_char_, 0x8001);
  this->current_operation_ = cover::COVER_OPERATION_IDLE;
  this->set_timeout("stop_moving_update", 200, [this]() { this->update_desk_data(); });
}

bool SmartIotLinak::is_at_target_(float height) const {
  switch (this->current_operation_) {
    case cover::COVER_OPERATION_OPENING:
      return height >= this->height_target_;
    case cover::COVER_OPERATION_CLOSING:
      return height <= this->height_target_;
    case cover::COVER_OPERATION_IDLE:
    default:
      return true;
  }
}
#endif // ESP32