#include "SmartIotLed.hpp"

using namespace SmartIotInternals;

std::vector<LedObject*> SmartIotLed::objects;

SmartIotLed::SmartIotLed(const char* id,const char* name, const char* type)
    :SmartIotNode(id,name,type)
    ,_milli_amps(4000)
    ,_fps(60)
    {
    setRunLoopDisconnected(true);
    setHandler([=](const String& json){
        return this->SmartIotLed::ledCmdHandler(json);
        });
}

void SmartIotLed::begin(){
    fill_solid(_leds,_nbLed, CRGB::Black);
}


void SmartIotLed::setup(){
    FastLED.setDither(false);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(100);
    //FastLED.setMaxPowerInVoltsAndMilliamps(5, _milli_amps);

    _nbLed = 0;
    for (LedObject* iObj : SmartIotLed::objects) {
        _nbLed = std::max(iObj->_firstPos + iObj->_nbLed ,static_cast<int>(_nbLed));
    }
    _leds =new CRGB [_nbLed];

    _display.attach_ms_scheduled(1000/_fps,std::bind(&SmartIotLed::display, this));
    //_display.attach_ms(1000/_fps,std::bind(&SmartIotLed::display, this));

    Interface::get().getLogger() << F(" Led node setuped (") << _nbLed << F(" leds)") << endl;

    advertise("state").setName("state").setRetained(true).setDatatype("integer").settable([=](const SmartIotRange& range, const String& value){
        return this->ledCmdHandler(range,value);
        });
}

void SmartIotLed::stop(){
    _display.detach();
}


LedObject* SmartIotLed::createObject(const uint8_t firstPos,const uint8_t nbled,const char* name) {
    LedObject* obj = new LedObject(firstPos,nbled,name);
    objects.push_back(obj);
    return obj;
}

bool SmartIotLed::ledCmdHandler(const SmartIotRange& range, const String& value){
    #ifdef DEBUG
        Interface::get().getLogger() << F("ledC node, handle action: ") << value << endl;
    #endif // DEBUG
    if (value == "100") { turnOn();}
    else if (value == "0") {turnOff();}
    else { return false; }
    return true;
}

bool SmartIotLed::ledCmdHandler(const String& json){
    DynamicJsonDocument parseJsonBuff (50+ JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(_nbObjects()) + (_nbObjects()) * ( JSON_OBJECT_SIZE(3)+JSON_ARRAY_SIZE(3))); 
    DeserializationError error = deserializeJson(parseJsonBuff, json);
    if (error) {
        Interface::get().getLogger() << F("✖ Invalid JSON LED commande: ") << error.c_str() << endl;
        return false;
    }
    JsonObject data = parseJsonBuff.as<JsonObject>();
    //serializeJsonPretty(data, Serial); 
    if(!data.containsKey("objects")) {   
        LedObject* obj = SmartIotLed::objects.front();
        SmartIotLed::ledObjCmdHandler(data,obj);
    } else {
        for (JsonPair item : data["objects"].as<JsonObject>()) {
            LedObject* obj = SmartIotLed::findObject(item.key().c_str());
            JsonObject objData = item.value().as<JsonObject>();
            if(obj) {SmartIotLed::ledObjCmdHandler(objData,obj);}
        }
    }
    return true;
}

void SmartIotLed::ledObjCmdHandler(ArduinoJson::JsonObject& data,LedObject* obj){
    if(data.containsKey("pattern")){ obj->setPattern(data["pattern"].as<String>()); }
    if(data.containsKey("speed")){ obj->setSpeed(data["speed"].as<uint8_t>()); }
    if(data.containsKey("rgb")){ obj->setColor(data["rgb"][0],data["rgb"][1],data["rgb"][2]);}  
}

void SmartIotLed::loop(){
    for (LedObject* iObj : SmartIotLed::objects) {
        iObj->audioLoop();
    }
}


void SmartIotLed::display(){
    bool show = false;
    random16_add_entropy(random(65535));
    for (LedObject* iObj : SmartIotLed::objects) {
        iObj->display();
        if( iObj->toShow()){
            show = true;
            for(int i = 0; i < iObj->_nbLed; ++i) {
                _leds[i+iObj->_firstPos] = iObj->_leds[i];
            } 
        }
    }
    if(show) {FastLED.show();}
    for (LedObject* iObj : SmartIotLed::objects) {iObj->showed();}
}

LedObject* SmartIotLed::findObject(const char* name){
        for (LedObject* iObj : SmartIotLed::objects) { 
            if (strcmp(name, iObj->getName()) == 0) { return iObj;}
        }
        return 0;
}

bool SmartIotLed::loadNodeConfig(ArduinoJson::JsonObject& data){
    SmartIotNode::loadNodeConfig(data);
    if(data.containsKey("fps")) {
        setFps(data["fps"]);
    }
    if(!data.containsKey("objects")) {
        if (data.containsKey("nb_led")){
            LedObject* obj = createObject(0,data["nb_led"],data["node_name"]);
            if(data.containsKey("audio_pin")) {
                obj->addAudio(data["audio_pin"]);  
            }
            if(data.containsKey("auto_play") && data.containsKey("auto_play_duration")) {
                obj->setAutoPlay(data["auto_play"].as<bool>(),data["auto_play_duration"].as<uint8_t>());  
            }
        } else {
            Interface::get().getLogger() << F("✖ Led config invalid: nb_led is missing") << endl;
        }
    } else {
        for (JsonPair item : data["objects"].as<JsonObject>()) {
            JsonObject objData = item.value().as<JsonObject>();
            if (objData.containsKey("nb_led") && objData.containsKey("name") && objData.containsKey("start_led") ) {
                LedObject* obj = createObject(objData["start_led"],objData["nb_led"],objData["name"].as<const char*>());
                if(objData.containsKey("audio_pin")) {
                    obj->addAudio(objData["audio_pin"]);  
                }
            } else {
                Interface::get().getLogger() << F("✖ Led config object invalid...") << endl;
            }
        }
    }
    return true;
}

void SmartIotLed::turnOff(){
    fill_solid(_leds,_nbLed, CRGB::Black);
    FastLED.show();
    _display.detach();
}

void SmartIotLed::turnOn(){
    _display.attach_ms_scheduled(1000/_fps,std::bind(&SmartIotLed::display, this));
}