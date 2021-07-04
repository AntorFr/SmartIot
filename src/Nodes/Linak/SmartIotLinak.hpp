#pragma once

#ifdef ESP32

#include <ArduinoJson.h>
#include <Ticker.h>
#include <SmartIot.h>

#include "FindDeskDevice.hpp"

#include <BLEDevice.h>

//#define DEBUG 1

class SmartIotLinak : public SmartIotNode  {
    public:
        SmartIotLinak(const char* id, const char* name, const char* type = "linak", const SmartIotInternals::NodeInputHandler& nodeInputHandler = [](const String& value) { return false; });
        ~SmartIotLinak();

        void set_mac_address(uint64_t address);
        void use_bluetooth_callback(bool bluetooth_callback) { bluetooth_callback_ = bluetooth_callback; };
        void set_ble_device(BLEAdvertisedDevice *device);

        void dump_config() override;

        void onConnect(BLEClient *p_client);
        void onDisconnect(BLEClient *p_client);
        void connect();

        void update_desk_data(uint8_t *pData = nullptr, bool allow_publishing_cover_state = true);
        cover::CoverTraits get_traits() override;
        void control(const cover::CoverCall &call) override;
        static IdasenDeskControllerComponent *instance;
        bool RequestHandler(const String& json);

    protected:
        virtual void setup() override;
        virtual void loop() override;
        virtual void onReadyToOperate() override;
        void publish_stats() override;
        virtual bool loadNodeConfig(ArduinoJson::JsonObject& data) override;

    private:
        sensor::Sensor *desk_height_sensor_ = nullptr;
        binary_sensor::BinarySensor *desk_moving_binary_sensor_ = nullptr;
        binary_sensor::BinarySensor *desk_connection_binary_sensor_ = nullptr;

        std::string ble_address_;
        bool bluetooth_callback_;
        BLEAdvertisedDevice *ble_device_ = nullptr;
        BLEClient *p_client_ = nullptr;
        BLERemoteCharacteristic *m_input_char_ = nullptr;
        BLERemoteCharacteristic *m_output_char_ = nullptr;
        BLERemoteCharacteristic *m_control_char_ = nullptr;

        float height_target_ = 0;
        cover::CoverOperation current_operation_{cover::COVER_OPERATION_IDLE};

        void scan_();
        void set_connection_(bool connected);

        void update_desk_();
        float get_heigth_();
        bool is_at_target_(float height) const;
        void publish_cover_state_(float position);

        void start_move_torwards_();
        void move_torwards_();
        void stop_move_();
        static ConfigValidationResult _validateConfig(const JsonObject object);
};
#endif // ESP32