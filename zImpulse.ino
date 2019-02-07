#ifdef Impulse

#include <Ticker.h>
 
volatile unsigned int Pulses = 0;
volatile unsigned int PulsesLast = 0;
volatile unsigned int PulsesKept = 0;

volatile unsigned int PulsesPeriods = 0;

//core on which the BLE detection task will run
static int taskCore = 0;

Ticker myTimer;
bool tickOccured;

void setupImpulse()
{
  pinMode(IMPULSEPIN, INPUT_PULLUP);
  attachInterrupt(IMPULSEPIN, pulseHandler, RISING);

  tickOccured = false;
  timerInit();
  
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
    trc(F("SmartIoT Impulse multicore ESP32 setup done "));
  #else
    trc(F("SmartIoT Impulse singlecore setup done "));
  #endif
}

volatile unsigned long LastMicros;

void pulseHandler() {
  if((long)(micros() - LastMicros) >= DEBOUNCE_MS * 1000) {
    Pulses = Pulses + 1;
    LastMicros = micros();
    //trc(F("Impulse detected"));
  }
}

void timerCallback() {
  PulsesLast = Pulses;
  PulsesKept += Pulses;
  Pulses = 0;
  
  PulsesPeriods++;

  tickOccured = true;
}

void timerInit(void) {
  myTimer.attach(IMPULSE_SAMPLE, timerCallback);
}

bool sendImpluse(){
    trc(F("Creating Impulse buffer"));
    StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
    JsonObject Impulsedata = jsonBuffer.to<JsonObject>();

    Impulsedata["impulse"] = PulsesKept/2; //(interupt raise for each edge > https://github.com/espressif/arduino-esp32/issues/1111 )
    Impulsedata["duration"] = PulsesPeriods * IMPULSE_SAMPLE;
    Impulsedata["periodes"] = PulsesPeriods;
    

    return pub(subjectImpulseoMQTT,Impulsedata);
  
  
 }

void coreTask( void * pvParameters ){
  String taskMessage = "Impulse Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();
  trc(taskMessage);
 
  while(true){

      //delay(TimeBtw_Read);

      if (tickOccured == true) {
        trc(F("Tick occured. Pulses kept so far: "));

        if (PulsesKept > 0) {
          if (sendImpluse()) {
            trc(F("Impulse MQTT publish ok"));
            PulsesKept = 0;
          }
        } else {
          PulsesPeriods = 0;
        }
        tickOccured = false;
      }
      vTaskDelay(10);
  }
}


#endif
