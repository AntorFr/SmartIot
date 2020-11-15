#include "SmartIotSwitch.hpp"

using namespace SmartIotInternals;


SmartIotSwitch::SmartIotSwitch(const char* id, const char* name, const char* type, const NodeInputHandler& inputHandler)
    :SmartIotNode(id,name,type,inputHandler)
    ,_pin(0)
    ,_debounceFlag(false) {
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

void SmartIotSwitch::setPin(uint8_t pin, bool defaultstate){
    _pin = pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, defaultstate);
    _state = defaultstate;
}

void SmartIotSwitch::impulse(uint16_t waveMs) {  
    if (!_debounceFlag) {
        _debounceFlag = true;
        _turn(!_state,false);
        _ticker.once_ms(waveMs,+[](SmartIotSwitch* Switch) { Switch->_turn(!(Switch->_state));}, this);
    }
}

void SmartIotSwitch::toggle() {  
    if (!_debounceFlag) {
        _debounceFlag = true;
        _turn(!_state,false);
        _ticker.once_ms(100,+[](SmartIotSwitch* Switch) { Switch->_unbounce();}, this);
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
