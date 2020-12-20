#include "SmartIotDoorCmd.hpp"

using namespace SmartIotInternals;


SmartIotDoorCmd::SmartIotDoorCmd(const char* id, const char* name, const char* type, const NodeInputHandler& inputHandler)
    :SmartIotNode(id,name,type,inputHandler)
    ,_pinOpen(0)
    ,_pinClose(0)
    ,_pinLight(0)
    ,_openDuration(30000)
    ,_closeDuration(30000)
    ,_switchOpen("switch1","Open")
    ,_switchClose("switch2","Close")
    ,_switchlight("light","light switch","light")
    ,_sensorToRead(false)
    ,_sensorActivated(true)
    ,_sensor()
    ,_status(0)
    ,_value(0),
    _lastMove(0) {
    setHandler([=](const String& json){
        return this->SmartIotDoorCmd::doorCmdHandler(json);
        });
    
}

SmartIotDoorCmd::~SmartIotDoorCmd() {
}

void SmartIotDoorCmd::setup() {
    Interface::get().getLogger() << F("• Setup Door Command node ") << getName() << endl;

    _switchOpen.notSettable();
    _switchClose.notSettable();

    _switchOpen.setPin(_pinOpen,_defaultPinState);
    _switchClose.setPin(_pinClose,_defaultPinState);

    if(_pinLight!=0){
        _switchlight.setName(this->getName());
        _switchlight.setPin(_pinLight,false);
    } else {
        _switchlight.notSettable();
    }

    if (_sensorActivated) {
            _sensorActivated = _initSensor(); // if no sensor detected, desable feature;
    }

    advertise("CurrentState").setName("currentstate").setRetained(true).setDatatype("string");
    advertise("TargetState").setName("targetstate").setRetained(false).setDatatype("string").settable([=](const SmartIotRange& range, const String& value){
        return this->SmartIotDoorCmd::doorCmdHandler(range,value);
        });
}

void SmartIotDoorCmd::onReadyToOperate() {
    if(_pinOpen == 0 || _pinClose == 0 ) { Interface::get().getLogger() << F("✖ Door Command node") << getName() << F(" pins not initialized")  << endl;}
    else { Interface::get().getLogger() << F("• Ready to operate Door Command node ") << getName() << endl;}

    if(_sensorActivated){
        _readSensor();
        _sensorTicker.attach(60,+[](SmartIotDoorCmd* Door) { Door->_readSensor();}, this);
    }

}

void SmartIotDoorCmd::loop() {
    if (_sensorToRead && _sensorActivated) {
      _statusSensor(_sensor.readRangeSingleMillimeters());
      if (_sensor.timeoutOccurred()) { 
          Interface::get().getLogger() << F(" ✖ Setup Door command : Sensor timeout!") << endl;
          _initSensor(); 
      }
    }
}

bool SmartIotDoorCmd::_initSensor(){
        _sensorMesures.clear();
        Wire.begin();
        _sensor.setTimeout(800);
        if (!_sensor.init())
        {
            Interface::get().getLogger() << F(" ✖ Setup Door command : Failed to detect and initialize sensor!") << endl;
            return false;
        } else {
            _sensor.setMeasurementTimingBudget(200000);
        }

        return true;
}

void SmartIotDoorCmd::_statusSensor(uint16_t mesure){
    _sensorMesures.push_back(mesure);
    if(_sensorMesures.size() <= 10){
        // not enought data to compute status yet
        return;
    } else {
        _sensorMesures.erase(_sensorMesures.begin(),_sensorMesures.begin()+_sensorMesures.size()-10);

        auto n = _sensorMesures.size();
        uint16_t avgMesure = std::accumulate(_sensorMesures.begin(), _sensorMesures.end(),0) / n;

        #ifdef DEBUG
            Interface::get().getLogger() << F("DoorCmd sensor avg value: ") << avgMesure << endl;

            DynamicJsonDocument jsonBuffer (JSON_OBJECT_SIZE(6)); 
            JsonObject data = jsonBuffer.to<JsonObject>();
            data["avgMesure"] = avgMesure;
            send(data);
        #endif // DEBUG



        if (avgMesure >= 200) {
            // door close
            _value = 0;
        } else if (avgMesure < 200)  {
            // door open
            _value = 100;
        }

        // sensor read properly, errase buffer and stop reading
        _sensorMesures.clear();
        _sensorToRead = false;
        _publishStatus();
    }


}

bool SmartIotDoorCmd::loadNodeConfig(ArduinoJson::JsonObject& data){
    SmartIotNode::loadNodeConfig(data);
    if (data.containsKey("pin_open") && data.containsKey("pin_close")) {
        setPins(data["pin_open"].as<uint8_t>(),data["pin_close"].as<uint8_t>());
    }
    if (data.containsKey("open_duration") && data.containsKey("close_duration")) {
        setDuration(data["open_duration"].as<uint32_t>(),data["close_duration"].as<uint32_t>());
    }
    if (data.containsKey("pin_light") && data.containsKey("light_duration")) {
       setupLight(data["pin_light"].as<uint32_t>(),data["light_duration"].as<uint32_t>());
    }
    if(data.containsKey("sensor") && data["sensor"].is<bool>()){
        activateSensor(data["sensor"].as<bool>());
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
                _endMove();
                _return = false;
            } else { // open in the midle and OnePin mode
                if (_lastMove == 2) {
                  _switchOpen.impulse(500);
                } else {
                  _switchOpen.multipleImpulse(3,500,800);
                }
                _startMove(1);
                _return = true;   
            }
            break;
        case 1: // oppenning
            // allready oppenning
            _endMove();
            _return = false;
            break;
        case 2: // closing
            _switchOpen.doubleImpulse(500,1000);
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
                _endMove();
                _return = false;
            } else { // open in the midle and OnePin mode
                if (_lastMove == 1) {
                  _switchClose.impulse(500);
                } else {
                  _switchClose.multipleImpulse(3,500,800);
                }
                _startMove(2);
                _return = true;   
            }
            break;
        case 1: // oppenning
            _switchClose.doubleImpulse(500,1000);
            _startMove(2);
            _return = true;
            break;
        case 2: // closing
            // allready closing
            _endMove();
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
            _endMove();
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
            _lastMove = _status;
            _ticker.once_ms(_openDuration,+[](SmartIotDoorCmd* Door) { Door->_endMove();}, this);
            _stopReadSensor();
            break;
        case 2: // closing
            _status = 2;
            _lastMove = _status;
            _ticker.once_ms(_closeDuration,+[](SmartIotDoorCmd* Door) { Door->_endMove();}, this);
            _stopReadSensor();
            break;  
        case 0: // stop
            _status = 0;
            _value = 50;
            _ticker.detach();
            _endMove();
            return;

        default:
            //wrong move
            return;          
    } 
    _publishStatus();
    if(_pinLight!=0){
        _switchlight.impulse((_lightDuration*1000));
    }
}
 
void SmartIotDoorCmd::_endMove(){
    if (_sensorActivated) { //sensor available > read position
        if(_status==0) {
            _publishStatus();
        } else {
            _status=0;
            _readSensor(true); // readSensor take care of the publication
        }
        
    } else { //sensor not available > deduce position
        switch(_status) {
            case 1: 
                _value = 100;
                break; //open
            case 2: 
                _value = 0;
                break; // close
            case 0:
                break; // not changing (or in the midle)
        }
        _status = 0;
        _publishStatus();
    }
}

void SmartIotDoorCmd::_publishStatus(){
    DynamicJsonDocument jsonBuffer (JSON_OBJECT_SIZE(6)); 
    JsonObject data = jsonBuffer.to<JsonObject>();

    switch(_status) {
        case 0:
            data["status"]= F("stopped");
            if (_value==0) {
                getProperty("CurrentState")->send("closed");
                data["state"] = F("closed");
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
    data["sensor"]= _sensorActivated;

    #ifdef DEBUG
        Interface::get().getLogger() << F(">DoorCmd node, status;") << endl;
        Interface::get().getLogger() << F("  - value: ") << _value << endl;
        Interface::get().getLogger() << F("  - status: ") << _status << endl;
        Interface::get().getLogger() << F("  - sensor: ") << _sensorActivated << endl;
    #endif // DEBUG

    send(data);

}






