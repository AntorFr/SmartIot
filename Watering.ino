#ifdef Watering

#include <AsyncTimer.h>

AsyncTimer WaterTimer(30000, Watering_off, Watering_on); 
void WateringtoMQTT() {
  WaterTimer.checkExpiration();
}

void setupWatering(){
  trc(F("WaterPIN"));
  trc(WaterPIN);
  //init
  pinMode(WaterPIN,OUTPUT);
  trc(F("Set to OFF"));
  digitalWrite(WaterPIN, LOW);
  trc(F("Watering setup done "));
}

void Watering_on(){
  WateringCmd(true);
}

void Watering_off(){
  WateringCmd(false);
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
      }
    }
  }
#endif

#endif
