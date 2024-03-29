#pragma once

#include <ArduinoJson.h>
#include <Ticker.h>
#include <SmartIot.h>
#include <Nodes/SmartIotSwitch.h>

#include <vector>
#include <numeric>

#include <Wire.h>

#include <VL53L0X.h>

//#define DEBUG 1

/*
TODO:
- Persiste curent "value" over reboot / shutdown
- Compute midle state value more accuratly based on time
*/


class SmartIotDoorCmd : public SmartIotNode  {
    public:
    SmartIotDoorCmd(const char* id, const char* name, const char* type = "switch", const SmartIotInternals::NodeInputHandler& nodeInputHandler = [](const String& value) { return false; });
    ~SmartIotDoorCmd();
    void setPins(uint8_t openPin,uint8_t closePin, bool defaultstate= 0);
    void setupLight(uint8_t pin, uint32_t duration) {_pinLight = pin; _lightDuration= duration;}
    void setmesureLimitOpen(uint16_t mesureLimitOpen) {_mesureLimitOpen = mesureLimitOpen;}
    void setDuration(uint32_t openDuration,uint32_t closeDuration) {_openDuration = openDuration; _closeDuration = closeDuration;};
    void activateSensor(bool activate = true) { _sensorActivated = activate; }
    bool doorCmdHandler(const String& json);
    bool doorCmdHandler(const SmartIotRange& range, const String& value);
    bool open();
    bool close();
    bool stopMotion();
    bool setValue(uint8_t value);

    protected:
    virtual void setup() override;
    virtual void loop() override;
    virtual void onReadyToOperate() override;
    virtual bool loadNodeConfig(ArduinoJson::JsonObject& data) override;

    private:
    uint8_t _pinOpen;
    uint8_t _pinClose;
    uint8_t _pinLight;
    
    uint32_t _openDuration;
    uint32_t _closeDuration;
    uint32_t _lightDuration;
    uint8_t _status; // 0:stop 1:openning 2:clossing
    uint8_t _value; // 100:open 0:closed
    uint8_t _lastMove;
    uint16_t _mesureLimitOpen;
    bool _defaultPinState;

    uint16_t _avgMesure;

    void _startMove(uint8_t move);
    void _endMove();
    void _publishStatus();

    Ticker _ticker;

    VL53L0X _sensor;
    std::vector<uint16_t >_sensorMesures;
    void _statusSensor(uint16_t mesure);
    Ticker _sensorTicker;
    bool _sensorToRead;
    bool _sensorActivated; //sensor is activated by default but can be configure in settings
    void _readSensor(bool forced = false){ if (_sensorActivated && _status == 0 && (_value == 100 || _value == 0 || forced == true)) { _sensorToRead = true; }  }
    void _stopReadSensor();
    bool _initSensor();
    

    SmartIotSwitch _switchOpen;
    SmartIotSwitch _switchClose;
    SmartIotSwitch _switchlight;

};
