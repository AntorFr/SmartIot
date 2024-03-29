#pragma once

#include <map>
#include "SmartIotLed.hpp"
#include "LedPattern.hpp"

class SmartIotLed;

namespace SmartIotInternals {
    class LedPattern;
}

class LedObject {
    friend SmartIotLed;
    friend SmartIotInternals::LedPattern;
    public:
        LedObject(const uint16_t firstPos,const uint16_t nbLed,const char* name);
        void display();
        void dimAll(byte value);
        void setColor(CRGB color);
        void setColor(uint8_t r, uint8_t g, uint8_t b){setColor(CRGB(r,g,b));}
        CRGB getColor() const {return _color;}
        uint8_t getSpeed() const {return _speed;}
        CRGBPalette16 getCurrentPalette() const {return _gCurrentPalette;}
        CRGBPalette16 getTargetPalette() const {return _gTargetPalette;}
        void setSpeed(uint8_t speed);
        void setPattern(String motif);
        void turnOff();
        void turnOn();
        bool getState();
        void initPattern();
        void setAutoPlay(bool autoplay, uint8_t duration);
        void setAutoPlay(bool autoplay);
        void setPlayList(ArduinoJson::JsonArray playlist);
        String getMotif() {return _pattern;}

        const char* getName() const {return _name;}

        void addAudio(uint8_t pin){_audioPin=pin; pinMode(_audioPin, INPUT);}
        void audioLoop();
        uint16_t getAudio() const;

    protected:
        uint16_t _nbLed;
        CRGB* _leds;
        std::vector<String> _autoplayList;
        std::map<String, std::function<SmartIotInternals::LedPattern*(LedObject* ledObj)>> _patterns;

    private:
        const char* _name;
        uint16_t _firstPos;
        uint8_t _speed;
        bool _state;
        bool _autoplay;
        uint8_t _autoplayDuration; // seconds
        Ticker _autoPlayTicker;
        CRGB _color;
        CRGBPalette16 _gCurrentPalette;
        CRGBPalette16 _gTargetPalette;
        String _pattern;
        
        void chooseNextColorPalette();
        void _setRandomPattern();

        SmartIotInternals::LedPattern* _curentPattern;

        uint8_t _audioPin;
        uint16_t _volume;
        uint16_t _avgVolume;

        void show();
        void showed();
        bool toShow();
        void _publishStatus(ArduinoJson::JsonObject& data);
};




