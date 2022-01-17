#include "Led.hpp"

using namespace SmartIotInternals;

LedObject::LedObject(const uint16_t firstPos,const uint16_t nbLed, const char* name)
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
    ,_autoplayDuration(30)
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
        _patterns["halloween"] = [](LedObject* ledObj) -> LedPattern* {LedPattern* pat = new HeatMapPattern(ledObj,HeatColors_p, true); pat->addPowerCut(30); return pat;};
        
        //TwinklePattern
        _patterns["cloudTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinklePattern(ledObj,CloudColors_p); };
        _patterns["rainbowTwinkles"]= [](LedObject* ledObj) -> LedPattern* { return new TwinklePattern(ledObj,RainbowColors_p); };
        _patterns["rainbowGlitterTwinkles"]= [](LedObject* ledObj) -> LedPattern* { LedPattern* pat = new TwinklePattern(ledObj,RainbowColors_p); pat->addGlitter(30); return pat;};
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
                        "confetti","star","pride","sinelon","cloudTwinkles","rainbowTwinkles","rainbowGlitterTwinkles","snowTwinkles","incandescentTwinkles","redGreenWhiteTwinkles","hollyTwinkles","redWhiteTwinkles",
                        "blueWhiteTwinkles","fairyLightTwinkles","snow2Twinkles","iceTwinkles","retroC9Twinkles","partyTwinkles","forestTwinkles","lavaTwinkles","fireTwinkles","cloud2Twinkles","oceanTwinkles"
                        };



}















