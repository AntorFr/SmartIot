#pragma once

#include <ArduinoJson.h>
#include <Ticker.h>
#include <SmartIot.h>


class SmartIotSwitch : public SmartIotNode  {
    public:
    SmartIotSwitch(const char* id, const char* name, const char* type = "switch", const SmartIotInternals::NodeInputHandler& nodeInputHandler = [](const String& value) { return false; });
    ~SmartIotSwitch();
    void setPin(uint8_t pin, bool defaultstate= 0);
    void turnOn()   {_turn(true);}
    void turnOff()  {_turn(false);}
    void toggle();
    void impulse(uint16_t waveMs);
    void doubleImpulse(uint16_t waveMs,uint16_t waitMs);
    bool SwitchHandler(const String& json);

    protected:
    void setup();
    void loop();
    void onReadyToOperate();
    bool loadNodeConfig(ArduinoJson::JsonObject& data);

    private:
    uint8_t _pin;
    Ticker _ticker1;
    Ticker _ticker2;
    uint16_t _doubleWaveMs;
    bool _debounceFlag;
    bool _state;
    void _turn(bool state,bool _debounce = true);
    void _unbounce(){_debounceFlag = false;}
};
