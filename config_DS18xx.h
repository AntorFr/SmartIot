#define TimeBetweenReadingOW 30000
#define subjectDS18XXtoMQTT  Base_Topic "sensors/" Gateway_Room "/" Gateway_Sensor "/temp" 
#define OWPIN D7

#define DS18XXNBSensor 1

//Time used to wait for an interval before resending measured values
//unsigned long timebh1750 = 0;

typedef struct  {
     String addr;
     float temp;
     unsigned long readtime;
} sensor_temp_struc;
