#ifdef TrainController

int TrainSpeed = 0; // set values you need to zero

//analogReadResolution(10);

int ConvertSpeed(int sensorValue)
{
  if (sensorValue>= 550) {
    sensorValue = map(sensorValue, 550, 1024, 10, 55);
  } else {
   sensorValue = map(sensorValue, 0, 549, -55, -10);
  }
  if (abs(sensorValue)<=15) {
    sensorValue = 0;
  }
  return sensorValue;
}

void ReadSensor() {
  int NewSpeed = ConvertSpeed(analogRead(A0));
  trc(NewSpeed);
  if (abs(TrainSpeed-NewSpeed)>=2) {
    TrainSpeed = NewSpeed;
    SendSpeedtoMQTT(TrainSpeed);
  }
  delay(10);
  
}

bool SendSpeedtoMQTT(int Newspeed){
    trc(F("Creating TrainMQTT buffer"));
    StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
    JsonObject jsondata = jsonBuffer.to<JsonObject>();
    jsondata["speed"] = Newspeed;
    return pub(tolower("smallcity/commands/train"),jsondata);
}


#endif
