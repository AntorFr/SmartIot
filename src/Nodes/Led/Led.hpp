#pragma once

#include <map>
#include "SmartIotLed.hpp"

class SmartIotLed;

class LedObject;

class LedMotif {
    friend LedObject;

    public:
    LedMotif(const LedObject* obj) {_obj=obj;}

    protected:
        virtual bool init();
        virtual bool display();

    private:
        const LedObject* _obj;
};

class LedObject {
    friend SmartIotLed;
    public:
    LedObject(const uint8_t nbLed,const uint8_t firstPos,const char* name);
    void display();
    //void heatMap(CRGBPalette16 palette, bool up);
    //void heatMap(led_obj obj,CRGBPalette16 palette, bool up);
    //void addGlitter(led_obj obj, uint8_t chanceOfGlitter);
    //void colorwaves( CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette);
    //void palettetest( CRGB* ledarray, uint16_t numleds, const CRGBPalette16& gCurrentPalette);
    void dimAll(byte value);
    void setColor(CRGB color){_color=color;}
    void setColor(uint8_t r, uint8_t g, uint8_t b){_color=CRGB(r,g,b);}
    void setSpeed(uint8_t speed) {_speed = speed;}
    void setMotif(String motif) {_motifInit=false; _motif = motif;}
    String getMotif() {return _motif;}
    const char* getName() const {return _name;}
    void setSolidColor(CRGB color);
    void setSolidColor(uint8_t r, uint8_t g, uint8_t b);

    private:
    const char* _name;
    uint8_t _firstPos;
    uint8_t _nbLed;
    CRGB* _leds;
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
    bool _show;
    CRGB _color;
    String _motif;
    CRGBPalette16 _gCurrentPalette;
    void show() {_show =true;}
    void showed() {_show =false;}
    bool _motifInit;

    std::map<String, void (LedObject::*)()> _patterns;

    void solidColor();
    void off();
    void blink();
    void blink1();
};




