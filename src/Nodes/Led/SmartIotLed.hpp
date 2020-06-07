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
    SmartIotLed(const char* id, const char* name, const char* type, const SmartIotInternals::NodeInputHandler& nodeInputHandler = [](const String& value) { return false; });

    LedObject* createObject(const uint8_t nbled, const uint8_t firstPos,const char* name);
    void setFps(uint8_t fps){_fps=fps;}

    static LedObject* findObject(const char* name);
    static std::vector<LedObject*> objects;

    template<uint8_t PIN,EOrder RGB_ORDER = GRB>
    void addLeds(){
        begin();
        FastLED.addLeds<WS2812,PIN, RGB_ORDER>(_leds, _nbLed); //GRB RGB //<NEOPIXEL,DATA_PIN, GRB>
    }

    protected:
    void setup();
    //void loop();
    //void onReadyToOperate();

    private:
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