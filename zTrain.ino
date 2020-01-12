#ifdef Train

#include <Servo.h>

Servo ESC; 
int TrainSpeed = 90; // set values you need to zero

int TrainSetSpeed(int NewSpeed)
{
    //transform -100 / 100 to 0 - 180
    int escspeed = (NewSpeed*90/100)+90;
    ESC.write(escspeed);
    return escspeed;
}

void MQTTtoTrain(char * topicOri, JsonObject& Data){
      String topic = topicOri;
      if (topic == tolower(subjectMQTTtoTrain) || topic == tolower(subjectMQTTtoAllTrain)) { 
        trc(F("New Train commande received"));
        int newSpeed = Data["speed"];
        int escSpeed = TrainSetSpeed(newSpeed);
        Data["esc_speed"] = escSpeed;
        trc(F("Train configured"));
        pub(tolower(subjecTraintoMQTT), Data);
      }
}

bool TraintoMQTT(char* message){
    String msg = message;
    trc(F("Creating TrainMQTT buffer"));
    StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
    JsonObject Traindata = jsonBuffer.to<JsonObject>();
    Traindata["log"] = msg;
    return pub(tolower(subjecTraintoMQTT),Traindata);
}

void setupTrain() {
  // put your setup code here, to run once:
  ESC.attach(TrainPort,1000,2000);
  ESC.write(TrainSpeed);
  delay(2000);
  trc(F("Train initialized"));
  TraintoMQTT("Train initialized");
}



#endif
