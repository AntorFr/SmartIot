#ifdef RailSwitch

#include <ServoEasing.h>

#define subjecRailSwitchtoMQTT "smallcity/railswitch/states"
#define subjectMQTTtoRailSwitch "smallcity/railswitch/commands" 


//Servo ESC; 

ServoEasing servos[4] = {
  ServoEasing(PCA9685_DEFAULT_ADDRESS, &Wire),
  ServoEasing(PCA9685_DEFAULT_ADDRESS, &Wire),
  ServoEasing(PCA9685_DEFAULT_ADDRESS, &Wire),
  ServoEasing(PCA9685_DEFAULT_ADDRESS, &Wire),
};

int servpos[4]; 

int InitPos = 1; // set values you need to zero


int SwitchSetpos(int NewPos,int servo=0,int sspeed=100) //0-1
{
    //inverse curent pos
    if (NewPos == -1) {
      NewPos = (servpos[servo]+1)%2; 
    }
    //transform -100 / 100 to 0 - 180
    int servoPos = 180-140*NewPos;
    servos[servo].startEaseTo(servoPos, sspeed);
    servpos[servo]=NewPos;
    return servoPos;
}

void MQTTtoRailSwitch(char * topicOri, JsonObject& Data){
      String topic = topicOri;
      if (topic == tolower(subjectMQTTtoRailSwitch)) {
        trc(F("New railswitch commande received"));
        for (int i=0; i<Data["switchs"].size(); i++) {
          trc("======");
          int servo = Data["switchs"][i]["servo"];
          int newPos = Data["switchs"][i]["pos"];
          if(Data["switchs"][i].containsKey("speed")) {
            int sspeed = Data["switchs"][i]["speed"];
            int servoPos =SwitchSetpos(newPos,servo,sspeed);
          } else {
            int servoPos =SwitchSetpos(newPos,servo);
          }
          //Data["switchs"][i]["servo_pos"] = servoPos;
        }
        trc(F("railswitch configured"));
        pub(tolower(subjecRailSwitchtoMQTT), Data);
      }
}

bool RailSwitchtoMQTT(char* message){
    String msg = message;
    trc(F("Creating RailSwitchtoMQTT buffer"));
    StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
    JsonObject RailSwitchdata = jsonBuffer.to<JsonObject>();
    RailSwitchdata["log"] = msg;
    return pub(tolower(subjecRailSwitchtoMQTT),RailSwitchdata);
}

void setupRailSwitcht() {
  // put your setup code here, to run once:
  for (int i=0; i<ARRAYSIZE(servos); i++) {
    servos[i].setSpeed(10);
    servos[i].attach(i);
    SwitchSetpos(InitPos,i);
  }

  trc(F("RailSwitch initialized"));
  RailSwitchtoMQTT("RailSwitch initialized");
}

#endif
