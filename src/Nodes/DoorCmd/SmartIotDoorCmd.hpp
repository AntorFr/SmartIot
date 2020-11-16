#pragma once

#include <ArduinoJson.h>
#include <Ticker.h>
#include <SmartIot.h>
#include <Nodes/SmartIotSwitch.h>


class SmartIotDoorCmd : public SmartIotNode  {
    public:
    SmartIotDoorCmd(const char* id, const char* name, const char* type = "switch", const SmartIotInternals::NodeInputHandler& nodeInputHandler = [](const String& value) { return false; });
    ~SmartIotDoorCmd();
    void setPins(uint8_t openPin,uint8_t closePin, bool defaultstate= 0);
    void setDuration(uint16_t openDuration,uint16_t closeDuration) {_openDuration = openDuration; _closeDuration = closeDuration;};
    bool doorCmdHandler(const String& json);
    bool open();
    bool close();
    bool stopMotion();
    bool setValue(uint8_t value);

    protected:
    void setup();
    void loop();
    void onReadyToOperate();

    private:
    uint8_t _pinOpen;
    uint8_t _pinClose;
    uint16_t _openDuration;
    uint16_t _closeDuration;
    uint8_t _status; // 0:stop 1:openning 2:clossing
    uint8_t _value;
    bool _defaultPinState;

    void _startMove(uint8_t move);
    void _endMove();

    Ticker _ticker;

    SmartIotSwitch _switchOpen;
    SmartIotSwitch _switchClose;

};
