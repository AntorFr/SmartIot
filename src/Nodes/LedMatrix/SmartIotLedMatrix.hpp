#pragma once

#include <ArduinoJson.h>
#include <Ticker.h>
#define FASTLED_INTERNAL //avoid prama message
#define FASTLED_ALLOW_INTERRUPTS 0 // 0 avoid esp8266 flickering
#define INTERRUPT_THRESHOLD 1

//#FIXME ESP8266 V2.7.4 works great but V3.0.x have some missing display even with latest fastledlib

#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>
#include <algorithm>

#include <SmartIot.h>

//#include "GradientPalettes.hpp"
//#include "SmartIotNode.hpp"
#include "../../SmartIot/Datatypes/Interface.hpp"
#include "LedMatrixPattern.hpp"


using namespace SmartIotInternals;

namespace SmartIotInternals {
    class LedMatrixPattern;
}

class SmartIotLedMatrix : public SmartIotNode  {
    friend SmartIotInternals::LedMatrixPattern;
    public:
    SmartIotLedMatrix(const char* id, const char* name, const char* type = "ledMatrix");

    void setFps(uint8_t fps){_fps=fps;}
    void setBrightness(uint8_t scale);
    uint8_t getBrightness() const {return _brightness;}
        
    bool getState();
    void setAutoPlay(bool autoplay, uint8_t duration);
    void setAutoPlay(bool autoplay);
    void setPlayList(ArduinoJson::JsonArray playlist);
    
    void setPattern(String motif);
    String getPattern() {return _pattern;}

    bool ledCmdHandler(const String& json);
    bool ledCmdHandler(const SmartIotRange& range,const String& value);
    bool SpeedCmdHandler(const SmartIotRange& range,const String& value);
    
    void turnOn();
    void turnOff();

    template<uint8_t PIN,EOrder RGB_ORDER = GRB, int16_t tWidth, int16_t tHeight, MatrixType_t tMType> 
    void init_matrix(){
        if (Interface::get().bootMode == SmartIotBootMode::NORMAL){ //config does not exist if boot mode is not normal.
            _matrix = new cLEDMatrix<tWidth, -tHeight, tMType>();
            FastLED.addLeds<WS2812,PIN, RGB_ORDER>(_matrix[0], _matrix.Size()); //GRB RGB //<NEOPIXEL,DATA_PIN, GRB>
            begin();
            FastLED.show();
            
        }
    }
    cLEDMatrix* getMatrix() {return _matrix;}

    protected:
        void setup() override;
        void loop() override;
        void stop() override;
        virtual bool loadNodeConfig(ArduinoJson::JsonObject& data) override;
        //void onReadyToOperate();
        void publish_stats() override;

        void initPattern();
        
        
    private:
        void ledObjCmdHandler(ArduinoJson::JsonObject& data,LedObject* obj,bool global = false);

        void _publishStatus();

        void begin();
        void display();
        uint16_t _milli_amps;  // 4000 IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
        uint8_t _fps;  // 60 here you can control the speed.
        Ticker _display;

        cLEDMatrix* _matrix;

        bool _state;
        uint8_t _brightness;

        bool _autoplay;
        uint8_t _autoplayDuration; // seconds
        std::vector<String> _autoplayList;
        Ticker _autoPlayTicker;

        
        void _setRandomPattern();
        String _pattern;

        SmartIotInternals::LedMatrixPattern* _curentPattern;
        std::map<String, std::function<SmartIotInternals::LedMatrixPattern*(LedObject* ledObj)>> _patterns;

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