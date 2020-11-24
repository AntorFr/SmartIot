#include "SmartIotDoorCmd.hpp"

using namespace SmartIotInternals;


SmartIotDoorCmd::SmartIotDoorCmd(const char* id, const char* name, const char* type, const NodeInputHandler& inputHandler)
    :SmartIotNode(id,name,type,inputHandler)
    ,_pinOpen(0)
    ,_pinClose(0)
    ,_openDuration(30000)
    ,_closeDuration(30000)
    ,_switchOpen("switch1","Open")
    ,_switchClose("switch2","Close")
    ,_status(0)
    ,_value(0),
    _lastMove(0) {
    setHandler([=](const String& json){
        return this->SmartIotDoorCmd::doorCmdHandler(json);
        });

    _switchOpen.notSettable();
    _switchClose.notSettable();
    
}

SmartIotDoorCmd::~SmartIotDoorCmd() {
}

void SmartIotDoorCmd::setup() {
    Interface::get().getLogger() << F("• Setup Door Command node ") << getName() << endl;

    _switchOpen.setPin(_pinOpen,_defaultPinState);
    _switchClose.setPin(_pinClose,_defaultPinState);

    advertise("CurrentState").setName("currentstate").setRetained(true).setDatatype("string");
    advertise("TargetState").setName("targetstate").setRetained(false).setDatatype("string").settable([=](const SmartIotRange& range, const String& value){
        return this->SmartIotDoorCmd::doorCmdHandler(range,value);
        });
}

void SmartIotDoorCmd::onReadyToOperate() {
    if(_pinOpen == 0 || _pinClose == 0 ) { Interface::get().getLogger() << F("✖ Door Command node") << getName() << F(" pins not initialized")  << endl;}
    else { Interface::get().getLogger() << F("• Ready to operate Door Command node ") << getName() << endl;}

}

void SmartIotDoorCmd::loop() {}

bool SmartIotDoorCmd::loadNodeConfig(ArduinoJson::JsonObject& data){
    SmartIotNode::loadNodeConfig(data);
    if (data.containsKey("pin_open") && data.containsKey("pin_close")) {
        SmartIotDoorCmd::setPins(data["pin_open"].as<uint8_t>(),data["pin_close"].as<uint8_t>());
    }
    if (data.containsKey("open_duration") && data.containsKey("close_duration")) {
        SmartIotDoorCmd::setDuration(data["open_duration"].as<uint16_t>(),data["close_duration"].as<uint16_t>());
    }
    return true;
}

void SmartIotDoorCmd::setPins(uint8_t openPin,uint8_t closePin, bool defaultstate){
    _pinOpen = openPin;
    _pinClose = closePin;
    _defaultPinState = defaultstate;
}

bool SmartIotDoorCmd::doorCmdHandler(const SmartIotRange& range, const String& value){
    #ifdef DEBUG
        Interface::get().getLogger() << F("DoorCmd node, handle action: ") << value << endl;
    #endif // DEBUG
    if (value == "open") { open();}
    else if (value == "close") {close();}
    else if (value == "stop") {stopMotion();}
    else { return false; }
    return true;
}

bool SmartIotDoorCmd::doorCmdHandler(const String& json){
    DynamicJsonDocument parseJsonBuff (5+ JSON_OBJECT_SIZE(2)); 
    DeserializationError error = deserializeJson(parseJsonBuff, json);
    if (error) {
        Interface::get().getLogger() << F("✖ Invalid JSON Door Command commande: ") << error.c_str() << endl;
        return false;
    }
    JsonObject data = parseJsonBuff.as<JsonObject>();
    //serializeJsonPretty(data, Serial); 

    if(data.containsKey("action")){ 
        #ifdef DEBUG
            Interface::get().getLogger() << F("DoorCmd node, handle action: ") << data["action"].as<const char*>() << endl;
        #endif // DEBUG

        if(data["action"].as<String>()== "open"){open();}
        if(data["action"].as<String>()== "close"){close();}
        if(data["action"].as<String>()== "stop"){stopMotion();}

        return true;
    }
    if(data.containsKey("value")){
        _value = data["value"].as<uint8_t>();
    } 

    return false;  

}

bool SmartIotDoorCmd::open(){
    #ifdef DEBUG
        Interface::get().getLogger() << F("DoorCmd node, commande open()") << endl;
    #endif // DEBUG
    bool _return = false;
    switch(_status) {
        case 0: // stop
            if (_value == 0 || (_pinOpen != _pinClose)){  // closed
                _switchOpen.impulse(500);
                _startMove(1);
                _return = true;
            } else if (_value == 100 ){  //allready open, do nothing
                _return = false;
            } else { // open in the midle
                if (_lastMove == 2) {
                  _switchOpen.impulse(500);
                } else {
                  _switchOpen.doubleImpulse(500,1000);
                }
                _startMove(2);
                _return = true;   
            }
            break;
        case 1: // oppenning
            // allready oppenning
            _return = false;
            break;
        case 2: // closing
            if (_pinOpen == _pinClose){
                _switchOpen.doubleImpulse(500,1000);
            } else {
                //_switchOpen.impulse(300);
                _switchOpen.doubleImpulse(500,1000);
            }
            _startMove(1);
            _return = true;
            break;
        default:
            _return = false;
            break;
    }
    return _return;
}


bool SmartIotDoorCmd::close(){
    #ifdef DEBUG
        Interface::get().getLogger() << F("DoorCmd node, commande close()") << endl;
    #endif // DEBUG
    bool _return = false;
    switch(_status) {
        case 0: // stop
            if (_value == 100 || (_pinOpen != _pinClose)){ // open
                _switchClose.impulse(500);
                _startMove(2);
                _return = true;
            } else if (_value == 0 ){  //allready closed, do nothing
                _return = false;
            } else { // open in the midle
                if (_lastMove == 1) {
                  _switchClose.impulse(500);
                } else {
                  _switchClose.doubleImpulse(500,1000);
                }
                _startMove(2);
                _return = true;   
            }
            break;
        case 1: // oppenning
            if (_pinOpen == _pinClose){
                _switchClose.doubleImpulse(500,1000);
            } else {
                //_switchClose.impulse(300);
                _switchClose.doubleImpulse(500,1000);
            }
            _startMove(2);
            _return = true;
            break;
        case 2: // closing
            // allready closing
            _return = false;
            break;
        default:
            _return = false;
            break;
    }
    return _return;
}

bool SmartIotDoorCmd::stopMotion(){
    #ifdef DEBUG
        Interface::get().getLogger() << F("DoorCmd node, commande stopMotion()") << endl;
    #endif // DEBUG
    switch(_status) {
        case 1: // oppenning
            _switchOpen.impulse(500);
            _startMove(0);
            return 1;
        case 2: // closing
            _switchClose.impulse(500);
            _startMove(0);
            return 1;
        case 0: // stop
            //Allready stoped
            return 0;
        default:
            return 0;
    }
    return 0;
}

void SmartIotDoorCmd::_startMove(uint8_t move){
    switch(move) {
        case 1: // oppenning
            _status = 1;
            _ticker.once_ms(_openDuration,+[](SmartIotDoorCmd* Door) { Door->_endMove();}, this);
            break;
        case 2: // closing
            _status = 2;
            _ticker.once_ms(_closeDuration,+[](SmartIotDoorCmd* Door) { Door->_endMove();}, this);
            break;  
        case 0:
            _lastMove = _status; // 1 oppenning - 2 closing
            _status = 0;
            _ticker.detach();
            _endMove();
            return;

        default:
            //wrong move
            return;          
    } 
    _publishStatus();
}
 
void SmartIotDoorCmd::_endMove(){
    switch(_status) {
        case 1: _value = 100; break; //open
        case 2: _value = 0; break; // close
        case 0: _value = 50; break; //somewhere in the midle 
    }
    _status = 0;
    _publishStatus();

}

void SmartIotDoorCmd::_publishStatus(){
    DynamicJsonDocument jsonBuffer (JSON_OBJECT_SIZE(5)); 
    JsonObject data = jsonBuffer.to<JsonObject>();

    switch(_status) {
        case 0:
            data["status"]= F("stopped");
            if (_value==0) {
                getProperty("CurrentState")->send("closed");
                data["state"] = F("close");
            } else if (_value==100) {
                data["state"] = F("open");
                getProperty("CurrentState")->send("open");
            }
            else {
                data["state"] = F("stopped");
                getProperty("CurrentState")->send("stopped");
            }
            break;
        case 1: 
            data["status"]= F("openning");
            data["state"] = F("openning");
            getProperty("CurrentState")->send("openning");
            getProperty("TargetState")->send("open");
            break;
        case 2: 
            data["status"]= F("closing"); 
            data["state"] = F("closing");
            getProperty("CurrentState")->send("closing");
            getProperty("TargetState")->send("close");
            break;
        
    }
    data["value"]= _value;

    send(data);

}






