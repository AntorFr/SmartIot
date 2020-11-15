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
    LedObject(const uint8_t firstPos,const uint8_t nbLed,const char* name);
    void display();
    //void heatMap(CRGBPalette16 palette, bool up);
    //void heatMap(led_obj obj,CRGBPalette16 palette, bool up);
    //void addGlitter(led_obj obj, uint8_t chanceOfGlitter);
    //void colorwaves( CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette);
    //void palettetest( CRGB* ledarray, uint16_t numleds, const CRGBPalette16& gCurrentPalette);
    void dimAll(byte value);
    void setColor(CRGB color);
    void setColor(uint8_t r, uint8_t g, uint8_t b){setColor(CRGB(r,g,b));}
    CRGB getColor() const {return _color;}
    uint8_t getSpeed() const {return _speed;}
    uint8_t getHue() const {return _gHue;}
    CRGBPalette16 getCurrentPalette() const {return _gCurrentPalette;}
    CRGBPalette16 getTargetPalette() const {return _gTargetPalette;}
    void setSpeed(uint8_t speed);
    void setPattern(String motif);
    String getMotif() {return _pattern;}
    const char* getName() const {return _name;}

    void addAudio(uint8_t pin){_audioPin=pin; pinMode(_audioPin, INPUT);}
    void audioLoop();
    uint16_t getAudio() const;

    protected:
    uint8_t _nbLed;
    CRGB* _leds;

    private:
    const char* _name;
    uint8_t _firstPos;
    uint8_t _speed;
    uint8_t _autoplay;
    uint8_t _autoplayDuration;
    unsigned long _autoPlayTimeout;
    uint8_t _gHue;
    CRGB _color;
    String _pattern;
    CRGBPalette16 _gCurrentPalette;
    CRGBPalette16 _gTargetPalette;
    uint8_t _currentPaletteIdx;
    
    void chooseNextColorPalette();

    SmartIotInternals::LedPattern* _curentPattern;

    uint8_t _audioPin;
    uint16_t _volume;
    uint16_t _avgVolume;



    void show();
    void showed();
    bool toShow();

    std::map<String, std::function<SmartIotInternals::LedPattern*(LedObject* ledObj)>> _patterns;
};




