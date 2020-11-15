#include "Led.hpp"

using namespace SmartIotInternals;

LedObject::LedObject(const uint8_t firstPos,const uint8_t nbLed, const char* name)
    :_nbLed(nbLed)
    ,_firstPos(firstPos)
    ,_speed(30)
    ,_gHue(0)
    ,_color(CRGB::Black)
    ,_gCurrentPalette(CRGB::Black)
    ,_name(name)
    ,_pattern('off')
    ,_curentPattern(nullptr)
    ,_audioPin(0)
    ,_volume(100)
    ,_avgVolume(100)
    ,_currentPaletteIdx(0)
    {
        _leds = new CRGB [_nbLed];
        fill_solid(_leds,_nbLed, CRGB::Black);

        _patterns["off"]= [](LedObject* ledObj) -> LedPattern* { return new OffPattern(ledObj); };
        _patterns["color"]= [](LedObject* ledObj) -> LedPattern* { return new ColorPattern(ledObj); };
        _patterns["blink"]= [](LedObject* ledObj) -> LedPattern* { return new BlinkPattern(ledObj); };
        _patterns["wipe"]= [](LedObject* ledObj) -> LedPattern* { return new WipePattern(ledObj); };
        _patterns["laser"]= [](LedObject* ledObj) -> LedPattern* { return new LaserPattern(ledObj); };
        _patterns["breathe"]= [](LedObject* ledObj) -> LedPattern* { return new BreathePattern(ledObj); };
        _patterns["rainbow"]= [](LedObject* ledObj) -> LedPattern* { return new RainbowPattern(ledObj); };
        _patterns["k2000"]= [](LedObject* ledObj) -> LedPattern* { return new K2000Pattern(ledObj); };
        _patterns["starTrek"]= [](LedObject* ledObj) -> LedPattern* { return new ComputerPattern(ledObj); };
        _patterns["rainbow sound"]= [](LedObject* ledObj) -> LedPattern* { return new RainbowSoundPattern(ledObj); };
        _patterns["confetti"]= [](LedObject* ledObj) -> LedPattern* { return new ConfettiPattern(ledObj); };
        _patterns["star"]= [](LedObject* ledObj) -> LedPattern* { return new StarPattern(ledObj); };
        _patterns["pride"]= [](LedObject* ledObj) -> LedPattern* { return new PridePattern(ledObj); };
    
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

void LedObject::setPattern(String pattern) {
    _pattern = pattern;
    if (_patterns.count(_pattern)>0) {
        _curentPattern = _patterns[_pattern](this);
        if (_curentPattern) {_curentPattern->init();}
    } else {
        Interface::get().getLogger() << F("✖ setPattern failed, Pattern ") << pattern.c_str() << F("does not exist") << endl;}
    }

void LedObject::display() { if(_curentPattern){_curentPattern->display();} }
void LedObject::showed() {if(_curentPattern){_curentPattern->showed();} }
void LedObject::show() { if(_curentPattern){_curentPattern->show();}}
bool LedObject::toShow(){
    if(_curentPattern){ return _curentPattern->toShow();} 
    else return false; 
}


void LedObject::audioLoop(){
    if(_audioPin){
        EVERY_N_MILLISECONDS(5){
        uint16_t aVol = 64*analogRead(_audioPin); // scale up from 0-1023 (10 bytes) to 0-65535 (16 bytes)
        _volume = (aVol * (1 - 0.92)) + (_volume * 0.92);
        }
        EVERY_N_MILLISECONDS(50){
        _avgVolume = (_volume * ( 100 - 99.8) + (_avgVolume * 99.8))/100;
        }
    }
    EVERY_N_MILLISECONDS(20) {_gHue++;}
}

uint16_t LedObject::getAudio() const{
    int16_t audioVol = 1024*_volume/((_avgVolume>0)?_avgVolume:1);
   // Interface::get().getLogger() << F("audioVol: ") << audioVol <<  F("  _avgVolume: ") << _avgVolume << endl;
    return audioVol;
}

void LedObject::chooseNextColorPalette()
{
  //const uint8_t numberOfPalettes = sizeof(gGradientPalettes) / sizeof(gGradientPalettes[0]);
  //_currentPaletteIdx = addmod8( _currentPaletteIdx, 1, numberOfPalettes);
  //_gTargetPalette = *(gGradientPalettes[_currentPaletteIdx]);
}
