#include "SmartIotLedMatrix.hpp"

using namespace SmartIotInternals;



SmartIotLedMatrix::SmartIotLedMatrix(const char* id,const char* name, const char* type)
    :SmartIotNode(id,name,type)
    ,_milli_amps(4000)
    ,_fps(60)
    ,_state(true)
    ,_brightness(100)
    ,_autoplay(false)
    ,_autoplayDuration(30)
    ,_curentPattern(nullptr)
    ,_pattern('off')
    {
    setRunLoopDisconnected(true);
    setHandler([=](const String& json){
        return this->SmartIotLedMatrix::ledCmdHandler(json);
        });
    setRetained(true);

    _patterns["off"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new OffPattern(matrix); };
    _patterns["blink"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new BlinkPattern(matrix); };
    _patterns["wipe"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new WipePattern(matrix); };
    _patterns["laser"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new LaserPattern(matrix); };
    _patterns["breathe"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new BreathePattern(matrix); };
    _patterns["rainbow"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new RainbowPattern(matrix); };
    _patterns["k2000"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new K2000Pattern(matrix); };
    _patterns["starTrek"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new ComputerPattern(matrix); };


    _patterns["confetti"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new ConfettiPattern(matrix); };
    _patterns["star"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new StarPattern(matrix); };
    _patterns["pride"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new PridePattern(matrix); };
    _patterns["sinelon"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new SinelonPattern(matrix); };

    //HeatMap
    _patterns["fire"] = [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new HeatMapPattern(matrix,HeatColors_p, true); };
    _patterns["water"] = [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new HeatMapPattern(matrix,IceColors_p, false); };
    _patterns["halloween"] = [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* {LedPattern* pat = new HeatMapPattern(matrix,HeatColors_p, true); pat->addPowerCut(30); return pat;};
    
    //TwinklePattern
    _patterns["cloudTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinklePattern(matrix,CloudColors_p); };
    _patterns["rainbowTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinklePattern(matrix,RainbowColors_p); };
    _patterns["rainbowGlitterTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { LedPattern* pat = new TwinklePattern(matrix,RainbowColors_p); pat->addGlitter(30); return pat;};
    _patterns["snowTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinklePattern(matrix,Snow2_p); };
    _patterns["incandescentTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinklePattern(matrix,FireOrange_p); };

    //TwinkleFoxPattern
    _patterns["redGreenWhiteTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,RedGreenWhite_p); };
    _patterns["hollyTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,Holly_p); };
    _patterns["redWhiteTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,RedWhite_p); };
    _patterns["blueWhiteTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,BlueWhite_p); };
    _patterns["fairyLightTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,FairyLight_p); };
    _patterns["snow2Twinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,Snow_p); };
    _patterns["iceTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,Ice_p); };
    _patterns["retroC9Twinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,RetroC9_p); };
    _patterns["partyTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,PartyColors_p); };
    _patterns["forestTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,ForestColors_p); };
    _patterns["lavaTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,LavaColors_p); };
    _patterns["fireTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,HeatColors_p); };
    _patterns["cloud2Twinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,CloudColors_p); };
    _patterns["oceanTwinkles"]= [](SmartIotLedMatrix* matrix) -> LedMatrixPattern* { return new TwinkleFoxPattern(matrix,OceanColors_p); };

    //default pattern on boot
    _pattern = "off";
    _curentPattern = _patterns["off"](this);

    _autoplayList = {
                    "confetti","star","pride","sinelon","cloudTwinkles","rainbowTwinkles","rainbowGlitterTwinkles","snowTwinkles","incandescentTwinkles","redGreenWhiteTwinkles","hollyTwinkles","redWhiteTwinkles",
                    "blueWhiteTwinkles","fairyLightTwinkles","snow2Twinkles","iceTwinkles","retroC9Twinkles","partyTwinkles","forestTwinkles","lavaTwinkles","fireTwinkles","cloud2Twinkles","oceanTwinkles"
                    };
}

void SmartIotLedMatrix::begin(){
    fill_solid(_matrix[0], _matrix.Size(), CRGB::Black);
}

void SmartIotLedMatrix::setup(){
    SmartIotNode::setup();
    
    FastLED.setDither(false);
    FastLED.setCorrection(TypicalLEDStrip);
    //FastLED.setMaxPowerInVoltsAndMilliamps(5, _milli_amps);

    _display.attach_ms_scheduled(1000/_fps,std::bind(&SmartIotLedMatrix::display, this));
    //_display.attach_ms(1000/_fps,std::bind(&SmartIotLed::display, this));

    Interface::get().getLogger() << F(" LedMatrix node setuped (") <<  << endl;

    advertise("state").setName("state").setRetained(true).setDatatype("integer").settable([=](const SmartIotRange& range, const String& value){
        return this->ledCmdHandler(range,value);
    });
}

void SmartIotLedMatrix::stop(){
    _display.detach();
}

void SmartIotLedMatrix::setBrightness(uint8_t scale){
    _brightness = scale;
    FastLED.setBrightness(dim8_raw(_brightness*255/100));
    FastLED.show();

}


bool SmartIotLedMatrix::ledCmdHandler(const SmartIotRange& range, const String& value){
    #ifdef DEBUG
        Interface::get().getLogger() << F("ledMatrix node, handle action: ") << value << endl;
    #endif // DEBUG
    int intValue = atoi(value.c_str());
    if(intValue<0 || intValue>100) return false; // brigthness between 0 - 100

    if (intValue == 0) { 
        turnOff();
        setBrightness(0);
    } else {
        turnOn();
        setBrightness(intValue);
    }
    _publishStatus();
    return true;
}

bool SmartIotLedMatrix::ledCmdHandler(const String& json){
    DynamicJsonDocument parseJsonBuff (50+ JSON_OBJECT_SIZE(10+_nbObjects()) + (_nbObjects()) * ( JSON_OBJECT_SIZE(10)+JSON_ARRAY_SIZE(3))); 
    DeserializationError error = deserializeJson(parseJsonBuff, json);
    if (error) {
        Interface::get().getLogger() << F("✖ Invalid JSON LED commande: ") << error.c_str() << endl;
        return false;
    }
    JsonObject data = parseJsonBuff.as<JsonObject>();
    if(data.containsKey("brightness")) {
        setBrightness(data["brightness"]);
    }
    if(data.containsKey("state")) {
        if(data["state"]=="OFF"){
            turnOff();
        } else if (data["state"]=="ON") {
            turnOn();
            if(getBrightness()==0) {setBrightness(100);}
        }
    }  
    if(data.containsKey("effect")){ 
        if (data["effect"] == "autoPlay") {
            obj->setAutoPlay(true);
        } else {
            obj->setPattern(data["effect"].as<String>());
            obj->setAutoPlay(false);
        }
    }
    if(data.containsKey("auto_play")){
        obj->setAutoPlay(data["auto_play"].as<bool>());
    }

    _publishStatus();
    return true;
}


void SmartIotLedMatrix::loop(){}


void SmartIotLedMatrix::display(){}

bool SmartIotLedMatrix::loadNodeConfig(ArduinoJson::JsonObject& data){
    SmartIotNode::loadNodeConfig(data);
    if(data.containsKey("fps")) {
        setFps(data["fps"]);
    }
    if(data.containsKey("brightness")) {
        setBrightness(data["brightness"]);
    } else {
        setBrightness(100);
    }

    if(data.containsKey("auto_play")) {
        if(data.containsKey("auto_play_duration")) obj->setAutoPlay(data["auto_play"].as<bool>(),data["auto_play_duration"].as<uint8_t>());
        else obj->setAutoPlay(data["auto_play"].as<bool>());
    }
    if(data.containsKey("play_list") && data["play_list"].is<JsonArray>()){
        obj->setPlayList(data["play_list"].as<JsonArray>());
    }
    
    return true;
}

void SmartIotLedMatrix::turnOff(){
    _state = false;
    _display.detach();
    fill_solid(_leds,_nbLed, CRGB::Black);
    FastLED.show();
}


void SmartIotLedMatrix::turnOn(){
    _state = true;
    for (LedObject* iObj : SmartIotLedMatrix::objects) {
        iObj->initPattern();
    }
    _display.attach_ms_scheduled(1000/_fps,std::bind(&SmartIotLedMatrix::display, this));
}

void SmartIotLedMatrix::_publishStatus(){
    DynamicJsonDocument jsonBuffer (JSON_OBJECT_SIZE(9) + _nbObjects() * JSON_OBJECT_SIZE(12)); 
    JsonObject data = jsonBuffer.to<JsonObject>();

    data[F("milli_amps")]=_milli_amps;
    data[F("fps")]=_fps;
    data[F("nb_led")]=_nbLed;
    data[F("brightness")]= getBrightness();
    data[F("state")]= _state?F("ON"):F("OFF");

    data[F("auto_play")] = _autoplay;
    data[F("effect")] = _pattern;

    JsonObject color = data.createNestedObject("color");
    color["r"] = _color.r;
    color["g"] = _color.g;
    color["b"] = _color.b;


    send(data);

    getProperty("state")->send(_state?String(getBrightness()):"0");
    getProperty("speed")->send(String(getSpeed()));
}

void SmartIotLedMatrix::publish_stats(){
    _publishStatus();
}

void SmartIotLedMatrix::setAutoPlay(bool autoplay, uint8_t duration) {
    _autoplayDuration=duration;
    setAutoPlay(autoplay);
}

void SmartIotLedMatrix::setPlayList(ArduinoJson::JsonArray playlist){
    _autoplayList.clear();
    for(JsonVariant v : playlist) {
        if(_patterns.count(v.as<String>())>0){
            _autoplayList.insert(_autoplayList.begin(),v.as<String>());
        } else {
            Interface::get().getLogger() << F("✖ setPlayList: ") << v.as<const char*>() << F(" pattern does not exist") << endl;
        }
    }
}

void SmartIotLedMatrix::setAutoPlay(bool autoplay) {
    _autoplay=autoplay;
    
    if(_autoplay) {
        _setRandomPattern();
        _autoPlayTicker.attach_scheduled(_autoplayDuration,std::bind(&LedObject::_setRandomPattern, this));
    } else {
         _autoPlayTicker.detach();
    }
}

void SmartIotLedMatrix::setPattern(String pattern) {
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

void SmartIotLedMatrix::_setRandomPattern(){    
    if(_state) {
        auto item = _autoplayList.begin();
        std::advance( item, random8(_autoplayList.size()-1));
        setPattern(*item);
    }
    
}


void SmartIotLedMatrix::initPattern(){
   if (_curentPattern) {_curentPattern->init();} 
}