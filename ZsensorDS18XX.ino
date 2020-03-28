#ifdef ZsensorDS18XX

#include <OneWire.h>
OneWire  ds(OWPIN);  // on pin 10 (a 4.7K resistor is necessary)

//Time used to wait for an interval before resending temp and hum
unsigned long timeOW = 0;

sensor_temp_struc sensor_temp[DS18XXNBSensor]; 

void MeasureTemp(){
  if (millis() > (timeOW + TimeBetweenReadingOW)) {//retriving value of temperature 

    trc(F("OneWire - Start reading"));
    timeOW = millis();
    
    byte data[12];
    byte addr[8];
    byte type_s;
    float temp;

    int nb_devices =0;
    
    ds.reset_search();

    while(ds.search(addr)) {
      nb_devices++;
      if (OneWire::crc8(addr, 7) != addr[7]) {
       trc(F("OneWire - CRC is not valid!"));
       continue;
      }

      switch (addr[0]) {
        case 0x10:
          trc(F("  Chip = DS18S20"));  // or old DS1820
          type_s = 1;
          break;
        case 0x28:
          trc(F("  Chip = DS18B20"));
          type_s = 0;
          break;
        case 0x22:
          trc(F("  Chip = DS1822"));
          type_s = 0;
          break;
        default:
          trc(F("Device is not a DS18x20 family device."));
          continue;
      }
      
      ds.reset();
      ds.select(addr);
      ds.write(0x44, 1);
      delay(1000);     // maybe 750ms is enough, maybe not
      
      ds.reset();
      ds.select(addr);
      ds.write(0xBE);

      for (byte i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
      }

      int16_t raw = (data[1] << 8) | data[0];
      if (type_s) {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
          // "count remain" gives full 12 bit resolution
          raw = (raw & 0xFFF0) + 12 - data[6];
        }
      } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        //// default is 12 bit resolution, 750 ms conversion time
      }
      
      temp = (float)raw / 16.0;
      String str_adress = String(addr[0],HEX)+String(addr[1],HEX)+String(addr[2],HEX)+String(addr[3],HEX)+String(addr[4],HEX)+String(addr[5],HEX)+String(addr[6],HEX)+String(addr[7],HEX);

      sensor_temp_struc last_read = {str_adress, temp,millis()};
      if (CompareToPrevious(last_read)) { 
        SendTempToMQTT(str_adress,temp);
      }
    }
    if (nb_devices==0) {
      trc(F("= No device founded"));
    }
  }
}

bool CompareToPrevious(sensor_temp_struc Sensor) {
  bool sensorfund = false;
  
  for (int i = 0; i < DS18XXNBSensor; i++) {
    if(sensor_temp[i].addr == Sensor.addr) {
      trc(F("Sensor fund - compare to previous"));
      sensorfund = true;
      if ((fabsf(sensor_temp[i].temp - Sensor.temp) >= 0.1) || (abs(sensor_temp[i].readtime - Sensor.readtime)> 5*60*1000)) {
         sensor_temp[i].temp = Sensor.temp;
         sensor_temp[i].readtime = Sensor.readtime;
         return true;
      } else {
         return false;
      }
    }
  }
  if (!sensorfund) {
      for (int i = 0; i < DS18XXNBSensor; i++) {
        trc(sensor_temp[i].addr);
        if(sensor_temp[i].addr== NULL) {
          trc(F("Sensor add to memory"));
          sensor_temp[i] = {Sensor.addr, Sensor.temp, Sensor.readtime};
          return true;
        }
      }      
  }
  trc(F("Error - Detected more device than declare... sensor ignored"));
  return false;
  
}

bool SendTempToMQTT(String addr, float temp){

    trc(F("Creating One Wire buffer"));
    StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
    JsonObject OWdata = jsonBuffer.to<JsonObject>();

    OWdata["adress"]=addr;
    OWdata["temp"]=temp;

    return pub(tolower(subjectDS18XXtoMQTT),OWdata);
 }

#endif
