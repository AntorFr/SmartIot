#include "Led.hpp"

using namespace SmartIotInternals;

LedObject::LedObject(const uint8_t nbLed,const uint8_t firstPos, const char* name)
    :_nbLed(nbLed)
    ,_firstPos(firstPos)
    ,_cooling(50)
    ,_sparking(60)
    ,_speed(30)
    ,_gCurrentPaletteNumber(0)
    ,_currentPatternIndex(0)
    ,_autoplayDuration(30)
    ,_autoplay(0)
    ,_autoPlayTimeout(0)
    ,_gHue(0)
    ,_color(CRGB::Black)
    ,_gCurrentPalette(CRGB::Black)
    ,_name(name)
    ,_motif('off')
    ,_show(false)
    ,_motifInit(false)
    {
        _leds = new CRGB [_nbLed];
        fill_solid(_leds,_nbLed, CRGB::Black);
        _patterns["color"] = &LedObject::solidColor;
        _patterns["off"] = &LedObject::off;
        _patterns["blink"] = &LedObject::blink;
        _patterns["blink1"] = &LedObject::blink1;

}


void LedObject::dimAll(byte value)
{
    for (int i = 0; i < _nbLed; i++) {
      _leds[i].nscale8(value);
    }
}


void LedObject::solidColor()
{
    if(!_motifInit){
    fill_solid(_leds,_nbLed, _color);
    show();
    _motifInit=true;
    }
}

void LedObject::off()
{
    if(!_motifInit){
    fill_solid(_leds,_nbLed, CRGB::Black);
    show();
    _motifInit=true;
    }

}

void LedObject::blink()
{
    EVERY_N_MILLISECONDS(100000/_speed)
    {
        Interface::get().getLogger() << F("Blink from ") << _name  << endl;
        bool off = (_leds[0]==static_cast<CRGB>(CRGB::Black));
        if (off) { fill_solid(_leds,_nbLed, _color);} 
        else { fill_solid(_leds,_nbLed, CRGB::Black);}
        show();
    }
}

void LedObject::blink1()
{
    EVERY_N_MILLISECONDS(100000/_speed)
    {
        Interface::get().getLogger() << F("Blink from ") << _name  << endl;
        bool off = (_leds[0]==static_cast<CRGB>(CRGB::Black));
        if (off) { fill_solid(_leds,_nbLed, _color);} 
        else { fill_solid(_leds,_nbLed, CRGB::Black);}
        show();
    }
}


void LedObject::display() {
    auto f = _patterns[_motif];
    (this->*f)();
}