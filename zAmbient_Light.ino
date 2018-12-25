#ifdef AmbientLight

#include "math.h" // Library for trig and exponential functions
#include "Wire.h" // Library for communication with I2C / TWI devices

void setupZAmbientLight()
{
  Wire.begin();
  Wire.beginTransmission(BH1750_i2c_addr);
  Wire.write(0x10);      // Set resolution to 1 Lux
  Wire.endTransmission();
  delay(300);
}
 
// void loop() 
void MeasureLightIntensity()
{
  if (millis() > (timebh1750 + TimeBetweenReadingBH1750)) {//retriving value of Lux, FtCd and Wattsm2 from BH1750
    trc(F("Creating BH1750 buffer"));
    StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
    JsonObject BH1750data = jsonBuffer.to<JsonObject>();
    
    timebh1750 = millis();
    unsigned int i=0;
    static float persistedll;
    static float persistedlf;
    static float persistedlw;
    unsigned int Lux;
    float ftcd;
    float Wattsm2;

    // Check if reads failed and exit early (to try again).
    Wire.requestFrom(BH1750_i2c_addr, 2);
    if(Wire.available() != 2) {
      trc(F("Failed to read from LightSensor BH1750!"));
    }else{
      i = Wire.read();
      i <<=8;
      i |= Wire.read(); 

      // Calculate the Values
      Lux = i/1.2;  // Convert to Lux
      ftcd = Lux/10.764;
      Wattsm2 = Lux/683.0;

      /*
      Useful Information ;-)
      lux (lx)                            # 1 lx = 1 lm/m² = 1 cd·sr·m⁻².
      meter-candle (m-cd)                 # 1 m·cd = 1 lx = 1 lm/m² = 1 cd·sr·m⁻².
      centimeter-candle (cm-sd)           # 1 m·cd = 1 lx = 1 lm/m² = 1 cd·sr·m⁻².
      foot-candle (ft-c)                  # 
      phot (ph)                           # 1 ph = 1 lm/cm² = 10,000 lm/m² - 10,000 lx = 10 klx
      nox (nx)                            # 1 nox = 1 millilux
      candela steradin/meter2(cd·sr·m⁻²)  # 1 lx = 1 lm/m² = 1 cd·sr·m⁻²
      lumen/meter2 (lm·m⁻²)               # 1 lx = 1 lm/m² = 1 cd·sr·m⁻²
      lumen/centimeter2 (lm·cm⁻²)         # 1 lm/cm² = 10,000 lx = 10,000 cd·sr·m⁻²
      lumen/foot2 (lm·ft⁻²)               # (lm·ft⁻²)
      watt/centimeter2 at 555nm  (W·cm⁻²) # 
      */

      // Generate Lux
      if(Lux != persistedll || bh1750_always){
        BH1750data["lux"] =  (unsigned int)Lux;
       }else{
        trc(F("Same lux don't send it"));
       }

      // Generate FtCd
      if(ftcd != persistedlf || bh1750_always){
        BH1750data["ftcd"] = (unsigned int)ftcd;
      }else{
        trc(F("Same ftcd don't send it"));
      }

      // Generate Watts/m2
      if(Wattsm2 != persistedlw || bh1750_always){
        BH1750data["wattsm2"] = (unsigned int)Wattsm2;
      }else{
        trc(F("Same wattsm2 don't send it"));
      }
      if(BH1750data.size()>0) pub(subjectBH1750toMQTT,BH1750data);
    }
    persistedll = Lux;
    persistedlf = ftcd;
    persistedlw = Wattsm2;
  }
}



#endif
