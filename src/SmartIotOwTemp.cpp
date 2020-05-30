#include "SmartIotOwTemp.hpp"

using namespace SmartIotInternals;


void OwTempDevice::setAddress(DeviceAddress add){
    snprintf(_address, (2*8+1)+1 , "%02x%02x%02x%02x%02x%02x", add[0], add[1], add[2], add[3], add[4], add[5],add[6],add[7]);
}

SmartIotOwTemp::SmartIotOwTemp(const char* id, const char* name, const char* type, const NodeInputHandler& inputHandler):SmartIotNode(id,name,type,inputHandler)
    ,_resolution(12)
    ,_nbDevices(0)
    ,_devices()
    ,_publicationPace(10)
    ,_pin(0)
    ,_WaitForConversionFlag(false) {
}

SmartIotOwTemp::~SmartIotOwTemp() {
}

void SmartIotOwTemp::setup() {
    Interface::get().getLogger() << F("• Setup OwTemp node ") << getName() << endl;
    _ds= OneWire(_pin);
    _sensors = DallasTemperature(&_ds);
    _sensors.begin();
    _nbDevices = _sensors.getDeviceCount();

    _sensors.setWaitForConversion(false); // makes it async
    Interface::get().getLogger() << _nbDevices << F(" device(s) found") << endl;

    for(uint8_t i=0;i<_nbDevices; i++){
        if(_sensors.getAddress(_tempDeviceAddress, i)) {
            OwTempDevice* device = new OwTempDevice(i,_tempDeviceAddress);
            _devices.push_back(device);
        }
    }
}

void SmartIotOwTemp::onReadyToOperate() {
    Interface::get().getLogger() << F("• Ready to operate OwTemp node ") << getName() << endl;
    //#ifdef ESP32
    _ticker.attach(_publicationPace,+[](SmartIotOwTemp* owTemp) { owTemp->_publishAllSensor();}, this);
    //#elif defined(ESP8266)
    //_ticker.attach(_publicationPace,_publishAllSensor);
    //#endif
}

void SmartIotOwTemp::loop() {  
    if (!_WaitForConversionFlag) {
    _WaitForConversionFlag = true;
    _sensors.requestTemperatures();
    //#ifdef ESP32
    _WaitForConversion.once_ms(_sensors.millisToWaitForConversion(_resolution),+[](SmartIotOwTemp* owTemp) { owTemp->_readSensor();}, this);
    //#elif defined(ESP8266)
    //_WaitForConversion.once_ms(_sensors.millisToWaitForConversion(_resolution), _readSensor);
    //#endif
    }
}

void SmartIotOwTemp::_readSensor() {
    for (OwTempDevice* iDevice : _devices) {
        float tempC =_sensors.getTempCByIndex(iDevice->getIndex());
        if (!_isValideTemp(tempC)) {
            Interface::get().getLogger() << F("x Error device") << iDevice->getAddress() << F(" not reachable") << endl;
            iDevice->setTempC(tempC);
        } else if (!(iDevice->isValideTemp()) || abs(tempC-iDevice->getTempC())>=0.2) {
            //Not yet initialized or not reached on last scan or difference > to threshold, publish it
            iDevice->setTempC(tempC);
            _publishSensor(iDevice);
        } else {
            // wait for recuring publish
            iDevice->setTempC(tempC);
        }
    }
    _WaitForConversionFlag = false;
}

void SmartIotOwTemp::_publishSensor(OwTempDevice* device) {
    DynamicJsonDocument jsonBuffer (JSON_OBJECT_SIZE(3)); 
    JsonObject data = jsonBuffer.to<JsonObject>();

    data["adress"]=device->getAddress();
    data["temp"]=device->getTempC();

    send(data);
    device->setPublish(true);
}

void SmartIotOwTemp::_publishAllSensor() {
    Interface::get().getLogger() << F("Publish sensor(s) data") << endl;
    for (OwTempDevice* iDevice : _devices) {
        if(!iDevice->getPublish()) {_publishSensor(iDevice);}
        iDevice->setPublish(false); //reset for next loop
    }
}