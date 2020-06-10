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
    LedObject(const uint8_t nbLed,const uint8_t firstPos,const char* name);
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
    void setSpeed(uint8_t speed);
    void setMotif(String motif);
    String getMotif() {return _motif;}
    const char* getName() const {return _name;}
    void setSolidColor(CRGB color);
    void setSolidColor(uint8_t r, uint8_t g, uint8_t b);


    protected:
    uint8_t _nbLed;
    CRGB* _leds;

    private:
    const char* _name;
    uint8_t _firstPos;
    uint8_t _patternIndex;
    uint8_t _currentPatternIndex;
    uint8_t _brightnessMap[5] = { 16, 32, 64, 128, 255 };
    //uint8_t _brightnessIndex = 0;
    //COOLING: How much does the air cool as it rises? 
    // Less cooling = taller flames.  More cooling = shorter flames. 
    // Default 50, suggested range 20-100
    uint8_t _cooling;  
    // SPARKING: What chance (out of 255) is there that a new spark will be lit?
    // Higher chance = more roaring fire.  Lower chance = more flickery fire.
    // Default 120, suggested range 50-200.
    uint8_t _sparking;
    uint8_t _speed;
    //TProgmemRGBGradientPalettePtr _gGradientPalettes[];
    uint8_t _gCurrentPaletteNumber;
    uint8_t _autoplay;
    uint8_t _autoplayDuration;
    unsigned long _autoPlayTimeout;
    uint8_t _gHue;
    CRGB _color;
    String _motif;
    CRGBPalette16 _gCurrentPalette;

    SmartIotInternals::LedPattern* _curentPattern;

    void show();
    void showed();
    bool toShow();

    std::map<String, std::function<SmartIotInternals::LedPattern*(LedObject* ledObj)>> _patterns;
};




