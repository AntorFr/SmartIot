#include "SmartIotLed.hpp"

using namespace SmartIotInternals;

std::vector<LedObject*> SmartIotLed::objects;

SmartIotLed::SmartIotLed(const char* id,const char* name, const char* type)
    :SmartIotNode(id,name,type)
    ,_milli_amps(4000)
    ,_fps(60)
    ,_state(true)
    ,_brightness(100)
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
    SmartIotNode::setup();
    
    FastLED.setDither(false);
    FastLED.setCorrection(TypicalLEDStrip);
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

void SmartIotLed::setBrightness(uint8_t scale){
    _brightness = scale;
    FastLED.setBrightness(dim8_raw(_brightness*255/100));
    FastLED.show();

}
uint8_t SmartIotLed::getBrightness(){
    return _brightness;
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

bool SmartIotLed::ledCmdHandler(const String& json){
    DynamicJsonDocument parseJsonBuff (50+ JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(_nbObjects()) + (_nbObjects()) * ( JSON_OBJECT_SIZE(3)+JSON_ARRAY_SIZE(3))); 
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
    _publishStatus();
    return true;
}

void SmartIotLed::ledObjCmdHandler(ArduinoJson::JsonObject& data,LedObject* obj){
    if(data.containsKey("effect")){ 
        if (data["effect"] == "autoPlay") {
            obj->setAutoPlay(true);
        } else {
            obj->setPattern(data["effect"].as<String>());
            obj->setAutoPlay(false);
        }
    }
    if(data.containsKey("speed")){ obj->setSpeed(data["speed"].as<uint8_t>()); }
    if(data.containsKey("color")){ obj->setColor(data["color"]["r"],data["color"]["g"],data["color"]["b"]);}  
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

    if(data.containsKey("brightness")) {
        setBrightness(data["brightness"]);
    } else {
        setBrightness(255);
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
                if(objData.containsKey("auto_play") && objData.containsKey("auto_play_duration")) {
                    obj->setAutoPlay(objData["auto_play"].as<bool>(),objData["auto_play_duration"].as<uint8_t>());  
                }
            } else {
                Interface::get().getLogger() << F("✖ Led config object invalid...") << endl;
            }
        }
    }
    return true;
}

void SmartIotLed::turnOff(){
    _state = false;
    fill_solid(_leds,_nbLed, CRGB::Black);
    FastLED.show();
    _display.detach();
}

void SmartIotLed::turnOn(){
    _state = true;
    _display.attach_ms_scheduled(1000/_fps,std::bind(&SmartIotLed::display, this));
}

void SmartIotLed::_publishStatus(){
    DynamicJsonDocument jsonBuffer (JSON_OBJECT_SIZE(6) + _nbObjects() * JSON_OBJECT_SIZE(6)); 
    JsonObject data = jsonBuffer.to<JsonObject>();

    data[F("milli_amps")]=_milli_amps;
    data[F("fps")]=_fps;
    data[F("nb_led")]=_nbLed;
    data[F("brightness")]= getBrightness();
    data[F("state")]= _state?F("ON"):F("OFF");

    if (_nbObjects()<= 1){
        LedObject* iLedObj = objects.front();
        iLedObj->_publishStatus(data);
    } else {
        for (LedObject* iLedObj : objects){
            JsonObject objData = data.createNestedObject(iLedObj->getName());
            iLedObj->_publishStatus(objData);
        }
    }
    send(data);

    getProperty("state")->send(String(getBrightness()));

}