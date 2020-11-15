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


//using namespace SmartIotInternals;
class LedObject;

class SmartIotLed : public SmartIotNode  {
    public:
    SmartIotLed(const char* id, const char* name, const char* type = "led");

    LedObject* createObject(const uint8_t firstPos,const uint8_t nbled,const char* name);
    void setFps(uint8_t fps){_fps=fps;}

    static LedObject* findObject(const char* name);
    static std::vector<LedObject*> objects;

    bool ledCmdHandler(const String& json);

    template<uint8_t PIN,EOrder RGB_ORDER = GRB>
    void addLeds(){
        begin();
        FastLED.addLeds<WS2812,PIN, RGB_ORDER>(_leds, _nbLed); //GRB RGB //<NEOPIXEL,DATA_PIN, GRB>
        FastLED.show();
    }



    protected:
    void setup() override;
    void loop() override;
    void stop() override;
    //void onReadyToOperate();
    uint8_t _nbObjects(){ return static_cast<uint8_t>(SmartIotLed::objects.size());} 

    private:
    void ledObjCmdHandler(ArduinoJson::JsonObject& data,LedObject* obj);

    void begin();
    void display();
    uint16_t _milli_amps;  // 4000 IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
    uint8_t _fps;  // 60 here you can control the speed. With the Access Point / Web Server the animations run a bit slower.
    Ticker _display;
    uint16_t _nbLed;
    CRGB* _leds;

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