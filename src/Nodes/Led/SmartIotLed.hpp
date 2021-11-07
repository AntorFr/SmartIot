#pragma once

#include <ArduinoJson.h>
#include <Ticker.h>
#define FASTLED_INTERNAL //avoid prama message
#define FASTLED_ALLOW_INTERRUPTS 0 //avoid esp8266 flickering
#include <FastLED.h>
#include <algorithm>

#include <SmartIot.h>

//#include "GradientPalettes.hpp"
//#include "SmartIotNode.hpp"
#include "../../SmartIot/Datatypes/Interface.hpp"
#include "Led.hpp"


using namespace SmartIotInternals;
class LedObject;

class SmartIotLed : public SmartIotNode  {
    public:
    SmartIotLed(const char* id, const char* name, const char* type = "led");

    LedObject* createObject(const uint8_t firstPos,const uint8_t nbled,const char* name);
    void setFps(uint8_t fps){_fps=fps;}
    void setBrightness(uint8_t scale);
    uint8_t getBrightness();

    static LedObject* findObject(const char* name);
    static std::vector<LedObject*> objects;

    bool ledCmdHandler(const String& json);
    bool ledCmdHandler(const SmartIotRange& range,const String& value);

    void turnOn();
    void turnOff();

    template<uint8_t PIN,EOrder RGB_ORDER = GRB> 
    void addLeds(){
        if (Interface::get().bootMode == SmartIotBootMode::NORMAL){ //config does not exist if boot mode is not normal.
            FastLED.addLeds<WS2812,PIN, RGB_ORDER>(_leds, _nbLed); //GRB RGB //<NEOPIXEL,DATA_PIN, GRB>
            begin();
            FastLED.show();
        }
    }

    protected:
        void setup() override;
        void loop() override;
        void stop() override;
        virtual bool loadNodeConfig(ArduinoJson::JsonObject& data) override;
        //void onReadyToOperate();
        void publish_stats() override;
        uint8_t _nbObjects(){ return static_cast<uint8_t>(SmartIotLed::objects.size());} 
        

    private:
        void ledObjCmdHandler(ArduinoJson::JsonObject& data,LedObject* obj);

        void _publishStatus();

        void begin();
        void display();
        uint16_t _milli_amps;  // 4000 IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
        uint8_t _fps;  // 60 here you can control the speed.
        Ticker _display;
        uint16_t _nbLed;
        CRGB* _leds;
        bool _state;
        uint8_t _brightness;

};

/*
enum EOrder {
	RGB=0012,
	RBG=0021,
	GRB=0102,
	GBR=0120,
	BRG=0201,
	BGR=0210
};
*/