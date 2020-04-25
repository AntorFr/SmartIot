#ifdef Watering

#include <AsyncTimer.h>

#ifdef watering_deep_sleep 
  #ifdef ESP32
  RTC_DATA_ATTR uint16_t bootCount = 0;
  #else
  
  #include <CRC.h>
  
  uint16_t bootCount = 0;
  struct {
    uint8_t crc8;
    uint16_t bootCount;
  } rtcBootCount;
  #endif
#endif

AsyncTimer WaterTimer(30000, Watering_off, Watering_on); 
void WateringtoMQTT() {
  WaterTimer.checkExpiration();
}

void setupWatering(){
  //bool waking_from_sleep = ESP.getResetReason() == "Deep-Sleep Wake"; //Detecting if boot from deep sleep
  
  trc(F("WaterPIN"));
  trc(WaterPIN);
  //init
  pinMode(WaterPIN,OUTPUT);
  trc(F("Set to OFF"));
  digitalWrite(WaterPIN, LOW);
  trc(F("Watering setup done "));

  #ifdef watering_deep_sleep
  countBOOT_SEQ();
  RequestWaterNeed();   //request for watering need
  #endif
  
}

void Watering_on(){
  WateringCmd(true);
}

void Watering_off(){
  WateringCmd(false);

  #ifdef watering_deep_sleep 
  Watering_deepsleep(); //goto deepsleep after turning off watering
  #endif
}

void WateringCmd(bool cmd){
  trc(F("Watering :"));
  trc(cmd);

  StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
  JsonObject WatData = jsonBuffer.to<JsonObject>();
  
  if (cmd) {
    digitalWrite(WaterPIN, HIGH);
    WatData["status"] = "ON";
  } else {
    digitalWrite(WaterPIN, LOW);
    WatData["status"] = "OFF";
  }  
  char JSONmessageBuffer[100];
  serializeJson(WatData, JSONmessageBuffer);
  pub(subjectWateringtoMQTT,JSONmessageBuffer);
}

#ifdef watering_deep_sleep 
void Watering_deepsleep(){
  
  trc(F("Setup SmartIot to sleep for (seconds) : "));
  trc(WATER_TIME_TO_SLEEP);

  digitalWrite(WaterPIN, LOW);
  GoToSleep(); //Warn MQTT
  delay(150);

  DesableNetwork(); //Shutdown Wifi and BT before going to sleep
  DeepSleep(WATER_TIME_TO_SLEEP);
}

void countBOOT_SEQ() {
  #ifdef ESP8266
    if (ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcBootCount, sizeof(rtcBootCount))) {
      uint8_t crcOfData = CRC::crc8((uint8_t*) &rtcBootCount.bootCount, sizeof(rtcBootCount.bootCount));
      if (crcOfData != rtcBootCount.crc8) {
        bootCount=0;
      } else {
        bootCount=rtcBootCount.bootCount;
      }
    }
    bootCount++;

    rtcBootCount.bootCount=bootCount;
    rtcBootCount.crc8 = CRC::crc8((uint8_t*) &rtcBootCount.bootCount, sizeof(rtcBootCount.bootCount));
    ESP.rtcUserMemoryWrite(0,(uint32_t*) &rtcBootCount, sizeof(rtcBootCount));
    
  #elif defined(ESP32)
    bootCount++;
  #endif

}
#endif //watering_deep_sleep

void RequestWaterNeed(){
  trc(F("RequestWateringNeed"));

  StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
  JsonObject ReqData = jsonBuffer.to<JsonObject>();

  ReqData["status"] = "Request";
  ReqData["subject"] = "Water_need";
  
  char JSONmessageBuffer[100];
  serializeJson(ReqData, JSONmessageBuffer);
  pub(subjectWateringtoMQTT,JSONmessageBuffer);

}

#ifdef watering_deep_sleep 
void GoToSleep(){
  trc(F("Warn GoToSleep"));

  StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
  JsonObject ReqData = jsonBuffer.to<JsonObject>();

  ReqData["status"] = "Sleep";
  ReqData["subject"] = "Go to sleep";
  ReqData["duration"] = WATER_TIME_TO_SLEEP;
  ReqData["nb_sleep"] = bootCount;
  
  char JSONmessageBuffer[100];
  serializeJson(ReqData, JSONmessageBuffer);
  pub(subjectWateringtoMQTT,JSONmessageBuffer);
}
#endif

  #ifdef simplePublishing
    void MQTTtoWatering(char * topicOri, char * datacallback){
    
      int duration;
      duration = atoi(datacallback);
      String topic = topicOri;
     
      if (topic == subjectMQTTtoWatering){
        trc(F("MQTTtoWatering duration :"));
        trc(duration);
        WaterTimer.kill();
        WaterTimer.config(duration * 1000, Watering_off, Watering_on);
        WaterTimer.start();
        // we acknowledge the sending by publishing the value to an acknowledgement topic
        pub(subjectWateringtoMQTT, datacallback);
      }
    }
  #endif
  
#ifdef jsonPublishing
  void MQTTtoWatering(char * topicOri, JsonObject& Wateringdata){
    String topic = topicOri;
    if (topic == subjectMQTTtoWatering){
      trc(F("MQTTtoWatering json data analysis"));
      int duration = Wateringdata["duration"];
      if (duration) {
        trc(F("MQTTtoWatering duration :"));
        trc(duration);
        WaterTimer.kill();
        WaterTimer.config(duration * 1000, Watering_off, Watering_on);
        WaterTimer.start();
        pub(subjectWateringtoMQTT, Wateringdata);
      }else{
        trc(F("MQTTtoONOFF Fail reading from json"));
        #ifdef watering_deep_sleep 
        Watering_deepsleep(); //goto deepsleep after turning off watering
        #endif
      }
    }
  }
#endif

#endif
