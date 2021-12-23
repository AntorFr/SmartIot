#include "Led.hpp"

using namespace SmartIotInternals;

LedObject::LedObject(const uint8_t firstPos,const uint8_t nbLed, const char* name)
    :_nbLed(nbLed)
    ,_firstPos(firstPos)
    ,_speed(30)
    ,_color(CRGB::White)
    ,_gCurrentPalette(CRGB::Black)
    ,_name(name)
    ,_pattern('off')
    ,_curentPattern(nullptr)
    ,_audioPin(0)
    ,_volume(100)
    ,_avgVolume(100)
    ,_autoplay(false)
    ,_state(true)
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

        //Sound
        _patterns["rainbow sound"]= [](LedObject* ledObj) -> LedPattern* { return new RainbowSoundPattern(ledObj); };

        _patterns["confetti"]= [](LedObject* ledObj) -> LedPattern* { return new ConfettiPattern(ledObj); };
        _patterns["star"]= [](LedObject* ledObj) -> LedPattern* { return new StarPattern(ledObj); };
        _patterns["pride"]= [](LedObject* ledObj) -> LedPattern* { return new PridePattern(ledObj); };
        _patterns["sinelon"]= [](LedObject* ledObj) -> LedPattern* { return new SinelonPattern(ledObj); };



        //HeatMap
        _patterns["fire"] = [](LedObject* ledObj) -> LedPattern* { return new HeatMapPattern(ledObj,HeatColors_p, true); };
        _patterns["water"] = [](LedObject* ledObj) -> LedPattern* { return new HeatMapPattern(ledObj,IceColors_p, false); };
        
        //TwinklePattern
        _patterns["cloudTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinklePattern(ledObj,CloudColors_p); };
        _patterns["rainbowTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinklePattern(ledObj,RainbowColors_p); };
        _patterns["snowTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinklePattern(ledObj,Snow2_p); };
        _patterns["incandescentTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinklePattern(ledObj,FireOrange_p); };

        //TwinkleFoxPattern
        _patterns["redGreenWhiteTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,RedGreenWhite_p); };
        _patterns["hollyTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,Holly_p); };
        _patterns["redWhiteTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,RedWhite_p); };
        _patterns["blueWhiteTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,BlueWhite_p); };
        _patterns["fairyLightTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,FairyLight_p); };
        _patterns["snow2Twinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,Snow_p); };
        _patterns["iceTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,Ice_p); };
        _patterns["retroC9Twinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,RetroC9_p); };
        _patterns["partyTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,PartyColors_p); };
        _patterns["forestTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,ForestColors_p); };
        _patterns["lavaTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,LavaColors_p); };
        _patterns["fireTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,HeatColors_p); };
        _patterns["cloud2Twinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,CloudColors_p); };
        _patterns["oceanTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinkleFoxPattern(ledObj,OceanColors_p); };

        //default pattern on boot
        _pattern = "color";
        _curentPattern = _patterns["color"](this);

        _autoplayList = {
                        "confetti","star","pride","sinelon","cloudTwinkles","rainbowTwinkles","snowTwinkles","incandescentTwinkles","redGreenWhiteTwinkles","hollyTwinkles","redWhiteTwinkles",
                        "blueWhiteTwinkles","fairyLightTwinkles","snow2Twinkles","iceTwinkles","retroC9Twinkles","partyTwinkles","forestTwinkles","lavaTwinkles","fireTwinkles","cloud2Twinkles","oceanTwinkles"
                        };



}

void LedObject::setColor(CRGB color){
     _color=color;
     initPattern();
}

void LedObject::setSpeed(uint8_t speed) {
    _speed = speed;
    initPattern();
}

void LedObject::initPattern(){
   if (_curentPattern) {_curentPattern->init();} 
}

void LedObject::dimAll(byte value)
{
    for (int i = 0; i < _nbLed; i++) {
      _leds[i].nscale8(value);
    }
}

void LedObject::_setRandomPattern(){    
    //auto item = _patterns.begin();
    //std::advance( item, random8(_patterns.size()-1));
    //setPattern(item->first);
    if(_state) {
        auto item = _autoplayList.begin();
        std::advance( item, random8(_autoplayList.size()-1));
        setPattern(*item);
    }
    
}

void LedObject::turnOff(){
    _state = false;
    delete _curentPattern;
    _curentPattern = _patterns["off"](this);
    initPattern();
    Interface::get().getLogger() << F("> turnOff: ") << F(" done") << endl;
}



bool LedObject::getState(){
    return _state;
}

void LedObject::turnOn(){
    _state = true;
    setPattern(_pattern);
    Interface::get().getLogger() << F("> turnOn: ") << F(" done") << endl;
}

void LedObject::setPattern(String pattern) {
    if(_state){
        if (_patterns.count(pattern)>0) {
            _pattern = pattern;
            delete _curentPattern;
            _curentPattern = _patterns[_pattern](this);
            Interface::get().getLogger() << F("> setPattern: ") << pattern.c_str() << F(" done") << endl;
            initPattern();
        } else {
            Interface::get().getLogger() << F("✖ setPattern failed, Pattern ") << pattern.c_str() << F(" does not exist") << endl;
        }
        
    }
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

void LedObject::setAutoPlay(bool autoplay, uint8_t duration) {
    _autoplay=autoplay;
    _autoplayDuration=duration;

    if(_autoplay) {
        _setRandomPattern();
        _autoPlayTicker.attach_scheduled(_autoplayDuration,std::bind(&LedObject::_setRandomPattern, this));
    } else {
         _autoPlayTicker.detach();
    }
}

void LedObject::setPlayList(ArduinoJson::JsonArray playlist){
    _autoplayList.clear();
    for(JsonVariant v : playlist) {
        if(_patterns.count(v.as<String>())>0){
            _autoplayList.insert(_autoplayList.begin(),v.as<String>());
        } else {
            Interface::get().getLogger() << F("✖ setPlayList: ") << v.as<const char*>() << F(" pattern does not exist") << endl;
        }
    }
}

void LedObject::_publishStatus(ArduinoJson::JsonObject& data){
    data[F("auto_play")] = _autoplay;
    data[F("effect")] = _pattern;
    data[F("speed")] = _speed;

    if(!data.containsKey("state")) {
        data[F("state")]= getState()?F("ON"):F("OFF");
    }
    //data[F("Color")] = "#"+ String((((long)_color.r << 16) | ((long)_color.g << 8 ) | (long)_color.b),HEX);

    JsonObject color = data.createNestedObject("color");
    color["r"] = _color.r;
    color["g"] = _color.g;
    color["b"] = _color.b;
    
    if(_audioPin){
        data[F("volume")] = _avgVolume;
    }
}
