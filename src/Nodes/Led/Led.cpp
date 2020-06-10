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
    ,_curentPattern(nullptr)
    {
        _leds = new CRGB [_nbLed];
        fill_solid(_leds,_nbLed, CRGB::Black);

        _patterns["off"]= [](LedObject* ledObj) -> LedPattern* { return new OffPattern(ledObj); };
        _patterns["color"]= [](LedObject* ledObj) -> LedPattern* { return new ColorPattern(ledObj); };
        _patterns["blink"]= [](LedObject* ledObj) -> LedPattern* { return new BlinkPattern(ledObj); };

}

 void LedObject::setColor(CRGB color){
     _color=color;
     if (_curentPattern) {_curentPattern->init();}
}

void LedObject::setSpeed(uint8_t speed) {
    _speed = speed;
    if (_curentPattern) {_curentPattern->init();}
}

void LedObject::dimAll(byte value)
{
    for (int i = 0; i < _nbLed; i++) {
      _leds[i].nscale8(value);
    }
}

void LedObject::setMotif(String motif) {
    //_motifInit=false; 
    _motif = motif;
    if (_patterns.count(_motif)>0) {
        _curentPattern = _patterns[_motif](this);
        if (_curentPattern) {_curentPattern->init();}
    } else {
        Interface::get().getLogger() << F("x SetMotif failed, Motif") << motif.c_str() << F("does not exist") << endl;}
    }

void LedObject::display() {
    /*
    auto f = _patterns[_motif];
    (this->*f)();
    */
     if(_curentPattern){_curentPattern->display();}

}

void LedObject::showed() {
    if(_curentPattern){_curentPattern->showed();}

}
void LedObject::show() {
    if(_curentPattern){_curentPattern->show();}
}
bool LedObject::toShow(){
    if(_curentPattern){ return _curentPattern->toShow();} 
    else return false; 
}