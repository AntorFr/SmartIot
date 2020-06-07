#include "SmartIotLed.hpp"

using namespace SmartIotInternals;

std::vector<LedObject*> SmartIotLed::objects;

SmartIotLed::SmartIotLed(const char* id,const char* name, const char* type, const SmartIotInternals::NodeInputHandler& inputHandler)
    :SmartIotNode(id,name,type,inputHandler)
    ,_milli_amps(4000)
    ,_fps(60)
    {
    setRunLoopDisconnected(true);
}

void SmartIotLed::begin(){
    _nbLed = 0;
    for (LedObject* iObj : SmartIotLed::objects) {
        _nbLed = std::max(iObj->_firstPos + iObj->_nbLed ,static_cast<int>(_nbLed));
    }
    _leds =new CRGB [_nbLed];

    fill_solid(_leds,_nbLed, CRGB::Black);
    FastLED.show();
}


void SmartIotLed::setup(){
    FastLED.setDither(false);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(100);
    //FastLED.setMaxPowerInVoltsAndMilliamps(5, _milli_amps);

    _display.attach_ms(1000/_fps,std::bind(&SmartIotLed::display, this));

    Interface::get().getLogger() << F(" Led node setuped (") << _nbLed << F(" leds)") << endl;
}


LedObject* SmartIotLed::createObject(const uint8_t nbled, const uint8_t firstPos,const char* name) {
    LedObject* obj = new LedObject(nbled,firstPos,name);
    objects.push_back(obj);
    return obj;
}


void SmartIotLed::display(){
    bool show = false;
    random16_add_entropy(random(65535));
    for (LedObject* iObj : SmartIotLed::objects) {
        iObj->display();
        if( iObj->_show){
            show = true;
            for(int i = 0; i < iObj->_nbLed; ++i) {
                _leds[i+iObj->_firstPos] = iObj->_leds[i];
            } 
        }
    }
    if(show) {FastLED.show();}
}

LedObject* SmartIotLed::findObject(const char* name){
        for (LedObject* iObj : SmartIotLed::objects) { 
            if (strcmp(name, iObj->getName()) == 0) { return iObj;}
        }
        return 0;
}

