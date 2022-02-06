#include "SmartIotLedMatrix.hpp"

using namespace SmartIotInternals;

LedObject* SmartIotLedMatrix::createObject(const uint16_t firstPos,const char* name){
    LedObject* obj = new LedMatrix(firstPos,name,_matrix);
    objects.push_back(obj);
    return obj;  
}

bool SmartIotLedMatrix::loadNodeConfig(ArduinoJson::JsonObject& data){
    SmartIotLed::loadNodeConfig(data);
    LedObject* obj = SmartIotLedMatrix::objects.front(); // matrix is always the first object
    if(data.containsKey("audio_pin")) {
        obj->addAudio(data["audio_pin"]);  
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



