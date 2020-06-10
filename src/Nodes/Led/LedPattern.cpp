#include "LedPattern.hpp"

using namespace SmartIotInternals;

LedPattern::LedPattern(const LedObject*  obj)
    :_show(false)
{
    _nbLed = obj->_nbLed;
    _leds = obj->_leds;
    _obj = obj;
}

void OffPattern::init(){
    fill_solid(_leds,_nbLed, CRGB::Black); 
    show();
}

void ColorPattern::init(){
    fill_solid(_leds,_nbLed, _obj->getColor()); 
    show();
}

void BlinkPattern::init(){
    _ticker.reset();
    _ticker.setPeriod(100000/_obj->getSpeed());
    show();
}

void BlinkPattern::display()
{
    if(_ticker)
    {
        //Interface::get().getLogger() << F("Blink from ") << _obj->getName()  << endl;
        if (_blink) { fill_solid(_leds,_nbLed, _obj->getColor());}
        else { fill_solid(_leds,_nbLed, CRGB::Black);}
        _blink= !_blink;
        show();
    }
}

