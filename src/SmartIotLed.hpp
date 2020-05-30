#pragma once

#include <ArduinoJson.h>
#include <Ticker.h>
#include "SmartIotNode.hpp"
#include "SmartIot/Datatypes/Interface.hpp"

class SmartIotLed;

namespace SmartIotInternals {
    class LedObject {
        friend ::SmartIotLed;
        public:

        private:

    };
} //end namespace SmartIotInternals


class SmartIotOwTemp : public SmartIotNode  {
    public:
    SmartIotOwTemp(const char* id, const char* name, const char* type, const SmartIotInternals::NodeInputHandler& nodeInputHandler = [](const String& value) { return false; });
    ~SmartIotOwTemp();

    protected:
    void setup();
    void loop();
    void onReadyToOperate();

    private:
    bool _isValideTemp(float tempC) { return (tempC != DEVICE_DISCONNECTED_C && tempC < 100 );}
    void _readSensor();
    void _publishSensor(SmartIotInternals::OwTempDevice* device);
    void _publishAllSensor();
    
    std::vector<SmartIotInternals::LedObject*> _objects;
};
