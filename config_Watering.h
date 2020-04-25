#define subjectWateringtoMQTT  Base_Topic "watering" 
#define subjectMQTTtoWatering  commands_Topic "watering"

#define WaterPIN D5    // Pin sur lequel est transistor pour la pompe

#define watering_deep_sleep "deep_sleep" /* uncomment for enable */
#define WATER_TIME_TO_SLEEP  10800        /* Time ESP32 will go to sleep (in seconds) */
