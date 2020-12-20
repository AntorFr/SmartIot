#include "SmartIotSwitch.hpp"

using namespace SmartIotInternals;


SmartIotSwitch::SmartIotSwitch(const char* id, const char* name, const char* type, const NodeInputHandler& inputHandler)
    :SmartIotNode(id,name,type,inputHandler)
    ,_pin(0)
    ,_debounceFlag(false)
    ,_numberImpulse(0) {
    setHandler([=](const String& json){
        return this->SmartIotSwitch::SwitchHandler(json);
    });
}

SmartIotSwitch::~SmartIotSwitch() {
}

void SmartIotSwitch::setup() {
    Interface::get().getLogger() << F("• Setup Switch node ") << getName() << endl;
}

void SmartIotSwitch::onReadyToOperate() {
    Interface::get().getLogger() << F("• Ready to operate Switch node ") << getName() << endl;
}

void SmartIotSwitch::loop() {}

bool SmartIotSwitch::loadNodeConfig(ArduinoJson::JsonObject& data){
    SmartIotNode::loadNodeConfig(data);
    if (data.containsKey("pin") ) {
        setPin(data["pin"].as<uint8_t>());
    }
    return true;
}

void SmartIotSwitch::setPin(uint8_t pin, bool defaultstate){
    _pin = pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, defaultstate);
    _state = defaultstate;
}



void SmartIotSwitch::impulse(uint32_t waveMs) {  
    _impulse(waveMs);
}

void SmartIotSwitch::_impulse(uint32_t waveMs) {  
    if (!_debounceFlag) {
        _debounceFlag = true;
        _turn(!_state,false);
        _ticker1.once_ms(waveMs,+[](SmartIotSwitch* Switch) { Switch->_turn(!(Switch->_state));}, this);
    }
}

void SmartIotSwitch::_nImpulse(uint32_t waveMs) { 
    if (_numberImpulse > 0) {
        --_numberImpulse;
        _impulse(waveMs);
    } else {
        _ticker2.detach();
    }
}

void SmartIotSwitch::doubleImpulse(uint32_t waveMs,uint32_t waitMs){ 
    /* 
    if (!_debounceFlag) {
        _debounceFlag = true;
        _turn(!_state,false);
        _waveMs = waveMs;
        _ticker1.once_ms(waveMs,+[](SmartIotSwitch* Switch) { Switch->_turn(!(Switch->_state));}, this);
        _ticker2.once_ms(waveMs+waitMs,+[](SmartIotSwitch* Switch) { Switch->impulse(Switch->_waveMs);}, this);
    }
    */
   multipleImpulse(2,waveMs,waitMs);

}

void SmartIotSwitch::multipleImpulse(uint8_t number,uint32_t waveMs,uint32_t waitMs){  
    if (number > 0) {
        _impulse(waveMs);
        _waveMs = waveMs;
        _numberImpulse = number-1;
        if (_numberImpulse>0) {
            _ticker2.attach_ms(waveMs+waitMs,+[](SmartIotSwitch* Switch) { Switch->_nImpulse(Switch->_waveMs);}, this);
        }
    }
}

void SmartIotSwitch::toggle() {  
    if (!_debounceFlag) {
        _debounceFlag = true;
        _turn(!_state,false);
        _ticker1.once_ms(100,+[](SmartIotSwitch* Switch) { Switch->_unbounce();}, this);
    }
}

void SmartIotSwitch::_turn(bool state,bool _debounce){
    #ifdef DEBUG
    Interface::get().getLogger() << F("> turn pin ") << _pin << F(" to ") << state << endl;
    #endif
    digitalWrite(_pin, state);
    _state = state;
    if (_debounce) {
        _debounceFlag = false;
    }
}

bool SmartIotSwitch::SwitchHandler(const String& json){
    DynamicJsonDocument parseJsonBuff (5+ JSON_OBJECT_SIZE(1)); 
    DeserializationError error = deserializeJson(parseJsonBuff, json);
    if (error) {
        Interface::get().getLogger() << F("✖ Invalid JSON Switch commande: ") << error.c_str() << endl;
        return false;
    }
    JsonObject data = parseJsonBuff.as<JsonObject>();
    //serializeJsonPretty(data, Serial); 

    if(data.containsKey("value")){ 
        if(data["value"].is<int>()){
            #ifdef DEBUG
                Interface::get().getLogger() << F("Switch node, handle value: ") << data["value"].as<int>() << endl;
            #endif // DEBUG

            if(data["value"]== 100) {SmartIotSwitch::turnOn();}
            if(data["value"]== 0) {SmartIotSwitch::turnOff();}
        }
        return true;
    }     
    return false;
}