#ifdef BT
  /*
     Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
     Ported to Arduino ESP32 by Evandro Copercini
  */
  // core task implementation thanks to https://techtutorialsx.com/2017/05/09/esp32-running-code-on-a-specific-core/

  #ifdef BLE
    #include <BLEDevice.h>
    #include <BLEUtils.h>
    #include <BLEScan.h>
    #include <BLEAdvertisedDevice.h>
  #endif

  #ifdef classicBT
    #include <BTDevice.h>
    #include <BTScan.h>
    #include <BTAdvertisedDevice.h>
  #endif
  
  //Time used to wait for an interval before resending BLE infos
  unsigned long timeBLE= 0;
    
  //core on which the BLE detection task will run
  static int taskCore = 0;

  #ifdef BLE
  class MyAdvertisedBLEDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
        void onResult(BLEAdvertisedDevice advertisedDevice) {
          trc(F("Creating BLE buffer"));
          StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
          JsonObject BLEdata = jsonBuffer.to<JsonObject>();
          String mac_adress = advertisedDevice.getAddress().toString().c_str();
          
          BLEdata["id"] = mac_adress;
          mac_adress.replace(":","");
          mac_adress.toUpperCase();

          BLEdata["bt_type"] = "BLE";

          String mactopic = subjectBTtoMQTT;
          mactopic.toLowerCase();
          mactopic = mactopic + mac_adress;
          
          if (advertisedDevice.haveName()){
              BLEdata["name"] = (char *)advertisedDevice.getName().c_str();
          }/*
          if (advertisedDevice.haveManufacturerData()){
              BLEdata["manufacturerdata"] = (char *)advertisedDevice.getManufacturerData().c_str();
              
          }*/
          if (advertisedDevice.haveRSSI()){
              BLEdata["rssi"] = (int)advertisedDevice.getRSSI();
              #ifdef subjectHomePresence
                haRoomPresence(BLEdata);// this device has an rssi in consequence we can use it for home assistant room presence component
              #endif
          }
          if (advertisedDevice.haveTXPower()){
              BLEdata["txpower"] = (int8_t) advertisedDevice.getTXPower();
          }          
          if (advertisedDevice.haveServiceData()){

              char mac[mac_adress.length()+1];
              mac_adress.toCharArray(mac,mac_adress.length()+1);
              
              trc(F("Get service data "));

              std::string serviceData = advertisedDevice.getServiceData();
              int serviceDataLength = serviceData.length();
              String returnedString = "";
              for (int i=0; i<serviceDataLength; i++)
              {
                int a = serviceData[i];
                if (a < 16) {
                  returnedString = returnedString + "0";
                } 
                returnedString = returnedString + String(a,HEX);  
              }
              
              char service_data[returnedString.length()+1];
              returnedString.toCharArray(service_data,returnedString.length()+1);
              service_data[returnedString.length()] = '\0';
              //BLEdata["servicedata"] = service_data;

              BLEdata["servicedatauuid"] =  (char *)advertisedDevice.getServiceDataUUID().toString().c_str();

              if(PubishRowServiceData) {
                pub((char *)mactopic.c_str(),BLEdata);
              }

              if (strstr(BLEdata["servicedatauuid"].as<char*>(),"fe95") != NULL) {
                trc(F("Processing BLE device data"));
                int pos = -1;  
                pos = strpos(service_data,"209800");
                trc(pos);
                if (pos != -1){
                  trc(F("mi flora data reading"));
                  //
                  #ifdef Mi_Flora
                    boolean result = processFloraDevice(advertisedDevice.getAddress(),mactopic,false);
                  #else
                    boolean result = process_data(pos - 24,service_data,mac);
                  #endif
                  //31029800009656678d7cc40d
                }
                pos = -1;
                pos = strpos(service_data,"20aa01");
                if (pos != -1){
                  trc(F("mi jia data reading"));
                  boolean result = process_data(pos - 26,service_data,mac);
                }

              }
          }
        }
    };
    #endif

    #ifdef classicBT
    class MyAdvertisedBTDeviceCallbacks: public BTAdvertisedDeviceCallbacks {
        void onResult(BTAdvertisedDevice advertisedDevice) {
          trc(F("Creating BT buffer"));
          StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
          JsonObject BTdata = jsonBuffer.to<JsonObject>();
          String mac_adress = advertisedDevice.getAddress().toString().c_str();
          
          BTdata["id"] = mac_adress;
          mac_adress.replace(":","");
          mac_adress.toUpperCase();

          BTdata["bt_type"] = "CBT";
          
          String mactopic = subjectBTtoMQTT + mac_adress;
          if (advertisedDevice.haveName()){
              BTdata["name"] = (char *)advertisedDevice.getName().c_str();
          }/*
          if (advertisedDevice.haveManufacturerData()){
              BTdata["manufacturerdata"] = (char *)advertisedDevice.getManufacturerData().c_str();
              
          }*/
          if (advertisedDevice.haveRSSI()){
              BTdata["rssi"] = (int)advertisedDevice.getRSSI();
              #ifdef subjectHomePresence
                haRoomPresence(BTdata);// this device has an rssi in consequence we can use it for home assistant room presence component
              #endif
          }
          if (advertisedDevice.haveTXPower()){
              BTdata["txpower"] = (int8_t) advertisedDevice.getTXPower();
          }          
        }
    };
    #endif

    void setupBT(){
        #ifdef multiCore
        // we setup a task with priority one to avoid conflict with other gateways
        xTaskCreatePinnedToCore(
                          coreTask,   /* Function to implement the task */
                          "coreTask", /* Name of the task */
                          10000,      /* Stack size in words */
                          NULL,       /* Task input parameter */
                          1,          /* Priority of the task */
                          NULL,       /* Task handle. */
                          taskCore);  /* Core where the task should run */
          trc(F("SmartIoT BT multicore ESP32 setup done "));
        #else
          trc(F("SmartIoT BT singlecore ESP32 setup done "));
        #endif
    }
    
    #ifdef multiCore
    void coreTask( void * pvParameters ){
        String taskMessage = "BT Task running on core ";
        taskMessage = taskMessage + xPortGetCoreID();
     
        while(true){
            trc(taskMessage);
            delay(TimeBtw_Read);

            #ifdef BLE
              BLEDevice::init(Gateway_Name);
              BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
              MyAdvertisedBLEDeviceCallbacks myBLECallbacks;
              pBLEScan->setAdvertisedDeviceCallbacks(&myBLECallbacks);
              pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
              BLEScanResults foundBLEDevices = pBLEScan->start(Scan_duration);
            #endif

            #ifdef classicBT
              BTDevice::init(Gateway_Name);
              BTScan* pBTScan = BTDevice::getScan(); //create new scan
              MyAdvertisedBTDeviceCallbacks myBTCallbacks;
              pBTScan->setAdvertisedDeviceCallbacks(&myBTCallbacks);
              BTScanResults foundBTDevices =pBTScan->start(Scan_duration); 
            #endif
        }
    }
    #else
    boolean BTtoMQTT(){
      unsigned long now = millis();
      if (now > (timeBLE + TimeBtw_Read)) {
              timeBLE = now;

              #ifdef BLE
                BLEDevice::init("");
                BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
                MyAdvertisedDeviceCallbacks myCallbacks;
                pBLEScan->setAdvertisedDeviceCallbacks(&myCallbacks);
                pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
                BLEScanResults foundDevices = pBLEScan->start(Scan_duration);
              #endif
              
              #ifdef classicBT
                BTDevice::init("");
                BTScan* pBTScan = BTDevice::getScan(); //create new scan
                MyAdvertisedBTDeviceCallbacks myBTCallbacks;
                pBTScan->setAdvertisedDeviceCallbacks(&myBTCallbacks);
                BTScanResults foundBTDevices =pBTScan->start(Scan_duration);
              #endif 
              return true;
      }
      return false;
    }
    #endif

String getBTAddress(){
  #ifdef classicBT
    return BTDevice::getAddress().toString().c_str();
  #elif defined (BLE)
    return BLEDevice::getAddress().toString().c_str();
  #endif    
}
      
boolean process_data(int offset, char * rest_data, char * mac_adress){
  trc(F("Creating BLE buffer"));
  StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
  JsonObject BLEdata = jsonBuffer.to<JsonObject>();
  trc("rest_data");
  trc(rest_data);
  int data_length = 0;
  switch (rest_data[51 + offset]) {
    case '1' :
    case '2' :
    case '3' :
    case '4' :
        data_length = ((rest_data[51 + offset] - '0') * 2)+1;
        trc("data_length");
        trc(data_length);
    break;
    default:
        trc("can't read data_length");
    return false;
    }
    
  char rev_data[data_length];
  char data[data_length];
  memcpy( rev_data, &rest_data[52 + offset], data_length );
  rev_data[data_length] = '\0';
  
  // reverse data order
  revert_hex_data(rev_data, data, data_length);
  double value = strtol(data, NULL, 16);
  trc(value);
  char val[12];

  String mactopic(mac_adress);
  mactopic = tolower(subjectBTtoMQTT) + mactopic;

  // second value
  char val2[12];
  trc("rest_data");
  trc(rest_data);
  // Mi flora provides tem(perature), (earth) moi(sture), fer(tility) and lux (illuminance)
  // Mi Jia provides tem(perature), batt(erry) and hum(idity)
  // following the value of digit 47 we determine the type of data we get from the sensor
  switch (rest_data[47 + offset]) {
    case '9' :
          dtostrf(value,0,0,val);
          BLEdata["fer"] = val;
    break;
    case '4' :
          if (value > 65000) value = value - 65535;
          dtostrf(value/10,3,1,val); // temp has to be divided by 10
          BLEdata["tem"] = val;
    break;
    case '6' :
          if (value > 65000) value = value - 65535;
          dtostrf(value/10,3,1,val); // hum has to be divided by 10
          BLEdata["hum"] = val;
    break;
    case '7' :
          dtostrf(value,0,0,val);
          BLEdata["lux"] = val;
    break;
    case '8' :
          dtostrf(value,0,0,val);
          BLEdata["moi"] = val;
    break;
    case 'a' : // batteryLevel
          dtostrf(value,0,0,val);
          BLEdata["batt"] = val;
    break;
    case 'd' : // temp+hum
          char tempAr[8];
          // humidity
          memcpy(tempAr, data, 4);
          tempAr[4] = '\0';
          value = strtol(tempAr, NULL, 16);
          if (value > 65000) value = value - 65535;
          dtostrf(value/10,3,1,val); // hum has to be divided by 10
          BLEdata["hum"] = val;
          // temperature
          memcpy(tempAr, &data[4], 4);
          tempAr[4] = '\0';
          value = strtol(tempAr, NULL, 16);
          if (value > 65000) value = value - 65535;
          dtostrf(value/10,3,1,val2); // hum has to be divided by 10
          BLEdata["tem"] = val2;
     break;
    default:
    trc("can't read values");
    return false;
    }

    pub((char *)mactopic.c_str(),BLEdata);
    return true;
}

void haRoomPresence(JsonObject& HomePresence){
  trc("BT/BLE DISTANCE :");
  double BLErssi = HomePresence["rssi"];
  double ratio = BLErssi/-59;
  double distance = (0.89)* pow(ratio,7.7095) + 0.11;  
  HomePresence["distance"] = distance;
  trc(distance);
  pub(tolower(subjectHomePresence),HomePresence);
}

#endif
