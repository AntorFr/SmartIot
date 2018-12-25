#define bh1750_always false // if false when the current value for light Level (Lux) is the same as previous one don't send it by MQTT
#define TimeBetweenReadingBH1750 30000

/*----------------------------USER PARAMETERS-----------------------------*/
/*-------------DEFINE YOUR MQTT PARAMETERS BELOW----------------*/

#define subjectBH1750toMQTT  Base_Topic "sensors/" Gateway_Room "/light" 

//Time used to wait for an interval before resending measured values
unsigned long timebh1750 = 0;

int BH1750_i2c_addr = 0x23; // Light Sensor I2C Address
