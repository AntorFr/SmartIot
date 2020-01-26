#ifdef TrainController

const uint8_t trainsCount = ARRAY_SIZE(trains);

//core on which the readsensor task will run
static int taskCore = 0;


void setupTrainController(){
  #ifdef ESP32
  analogReadResolution(12);
  #endif
  
  for(int i=0;i<trainsCount;i++){
    pinMode(trains[i].pin, OUTPUT);
  }
  
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
    trc(F("SmartIoT BT singlecore setup done "));
  #endif
}
    
#ifdef multiCore
void coreTask( void * pvParameters ){
    String taskMessage = "Sensor Task running on core ";
    taskMessage = taskMessage + xPortGetCoreID();
 
    while(true){
        //trc(taskMessage);
        ReadSensors();
    }
}

void loopTrainControler(){
  // do nothing all is handle by Core 2
}
#else
void loopTrainControler(){
  ReadSensors();
}
#endif

void ReadSensors() {
  for(int i=0;i<trainsCount;i++){
    ReadSensor(&trains[i]);
  }
  delay(5);
}


int ReadSensor(TrainConfig *train)
{
  int16_t sensorValue = analogRead(train->pin);

  if (sensorValue>= train->midle) {
    sensorValue = map(sensorValue, train->midle, 4095, 10, train->max);
  } else {
   sensorValue = map(sensorValue, 0, train->midle-1, train->min, -10);
  }
  if (abs(sensorValue)<=15) {
    sensorValue = 0;
  }

  if (abs(train->lastspeed-sensorValue)>=2) {
    trc(analogRead(train->pin));
    train->lastspeed = sensorValue;
    SendSpeedtoMQTT(train);
  }
}


bool SendSpeedtoMQTT(TrainConfig *train){
    trc(F("Creating TrainMQTT buffer"));
    StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
    JsonObject jsondata = jsonBuffer.to<JsonObject>();
    jsondata["speed"] = train->lastspeed;
    String topic = subjectMQTTtoControlTrain + train->name;
    topic.toLowerCase();
    return pub(topic,jsondata);
}


#endif
