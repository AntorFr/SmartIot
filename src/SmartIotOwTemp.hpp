#pragma once

#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Ticker.h>
#include "SmartIotNode.hpp"
#include "SmartIot/Datatypes/Interface.hpp"

class SmartIotOwTemp;

namespace SmartIotInternals {
    class OwTempDevice {
        friend ::SmartIotOwTemp;
        public:
            explicit OwTempDevice(uint8_t index,DeviceAddress deviceAddress ) {_index=index; _tempC = DEVICE_DISCONNECTED_C; _publish=false; setAddress(deviceAddress);}
            void setTempC(float tempC) { _tempC = tempC; }
            void setAddress(DeviceAddress add);
            void setPublish(bool publish) {_publish = publish;}
            bool isValideTemp() { return (_tempC != DEVICE_DISCONNECTED_C && _tempC < 100 );}
            const char* getAddress() const { return _address; }
            float getTempC() const { return _tempC; }
            uint8_t getIndex() const { return _index; }
            bool getPublish() const { return _publish; }

        private:
            char _address[16];
            uint8_t _index;
            float _tempC;
            bool _publish;
    };
} //end namespace SmartIotInternals


class SmartIotOwTemp : public SmartIotNode  {
    public:
    SmartIotOwTemp(const char* id, const char* name, const char* type, const SmartIotInternals::NodeInputHandler& nodeInputHandler = [](const String& value) { return false; });
    ~SmartIotOwTemp();
    void setPin(uint8_t pin) {_pin = pin;}

    protected:
    void setup();
    void loop();
    void onReadyToOperate();

    private:
    bool _isValideTemp(float tempC) { return (tempC != DEVICE_DISCONNECTED_C && tempC < 100 );}
    void _readSensor();
    void _publishSensor(SmartIotInternals::OwTempDevice* device);
    void _publishAllSensor();
    OneWire _ds;
    DallasTemperature _sensors;
    uint8_t _pin;
    Ticker _ticker;
    Ticker _WaitForConversion;
    float _publicationPace;
    uint8_t _resolution;
    uint8_t _nbDevices;
    DeviceAddress _tempDeviceAddress;
    const char* _tempAddress;
    bool _WaitForConversionFlag;

    std::vector<SmartIotInternals::OwTempDevice*> _devices;
};
