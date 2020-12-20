#pragma once

#include <ArduinoJson.h>
#include <Ticker.h>
#include <SmartIot.h>

//#define DEBUG 1

class SmartIotSwitch : public SmartIotNode  {
    public:
    SmartIotSwitch(const char* id, const char* name, const char* type = "switch", const SmartIotInternals::NodeInputHandler& nodeInputHandler = [](const String& value) { return false; });
    ~SmartIotSwitch();
    void setPin(uint8_t pin, bool defaultstate= 0);
    void turnOn()   {_turn(true);}
    void turnOff()  {_turn(false);}
    void toggle();
    void impulse(uint32_t waveMs); 
    void doubleImpulse(uint32_t waveMs,uint32_t waitMs);
    void multipleImpulse(uint8_t number,uint32_t waveMs,uint32_t waitMs);
    bool SwitchHandler(const String& json);

    protected:
    virtual void setup() override;
    virtual void loop() override;
    virtual void onReadyToOperate() override;
    virtual bool loadNodeConfig(ArduinoJson::JsonObject& data) override;

    private:
    uint8_t _pin;
    Ticker _ticker1;
    Ticker _ticker2;
    uint32_t _waveMs;
    uint8_t _numberImpulse;
    bool _debounceFlag;
    bool _state;
    void _turn(bool state,bool _debounce = true);
    void _unbounce(){_debounceFlag = false;}
    void _impulse(uint32_t waveMs);
    void _nImpulse(uint32_t waveMs); 
};
