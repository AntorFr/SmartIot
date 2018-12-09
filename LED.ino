#ifdef LED

  #include <Adafruit_NeoPixel.h>
  
  const float Led_density = round( 144 * 100 / LED_DENSITY )/100;
  const int Led_lenght = LED_COUNT / SYMETRICAL;
  
  unsigned long timeLED = 0; // When the sensor was last read   
  
  String motif = String(50);//LED status flag
  int ledspeed = 30;
  int RGB[3] = {0,0,0};
      
  #ifdef LED_AUDIO
    static float volume_moy= 1;
    unsigned long timeLED_AUDIO = 0;
  #endif
  
  void setupLED(){
    #ifdef LED_AUDIO
      pinMode(AUDIO_AIN, INPUT);
      pinMode(AUDIO_DIN, OUTPUT);
    #endif
  
    motif = "noir";
    all_off();
    
  }
  
  void MQTTtoLED(char * topicOri, JsonObject& LEDData){
      String topic = topicOri;
      if (topic == subjectMQTTtoWatering){
        const char* sensor = newjson["motif"];
        motif = String(sensor);
        ledspeed = int(newjson["speed"]);
        RGB[0] = newjson["RGB"][0];
        RGB[1] = newjson["RGB"][1];
        RGB[2] = newjson["RGB"][2];
        timeLED = 0;
        noir();
        pub(subjectWateringtoMQTT, Wateringdata);
      }
  }

  #ifdef LED_AUDIO
  void read_audio(){
     unsigned long now = millis();
      if (now > (timeLED_AUDIO + 5)) {
        timeLED_AUDIO = now;
        volume = analogRead(AUDIO_AIN);
        volume_filter = (volume * (1 - filterVal)) + (volume_filter * filterVal);
      }
  }
  #endif
  
  void display_led(){
    if (motif == "Laser") {
        Laser(strip.Color(RGB[0], RGB[1], RGB[2]), ledspeed);
    } else if (motif == "K2000") {
        K2000(RGB[0], RGB[1], RGB[2]);
    } else if (motif == "Blink") {
        Blink(RGB[0], RGB[1], RGB[2],ledspeed);
    } else if (motif == "StarTrek") {
        StarTrek(RGB[0], RGB[1], RGB[2]);
    } else if (motif == "Wipe") {
        Wipe(strip.Color(RGB[0], RGB[1], RGB[2]), ledspeed);
    } else if (motif == "Color") {
        color(strip.Color(RGB[0], RGB[1], RGB[2]));
    } else if (motif == "Clignote") {
        Clignote(strip.Color(RGB[0], RGB[1], RGB[2]),ledspeed);
    } else if (motif == "Variation") {
        Variation(ledspeed);
    } else if (motif == "Noel") {
        noel(ledspeed);
    } else if (motif == "Police") {
        police(ledspeed);
    } else if (motif == "Halloween") {
        candle(255,78,0,ledspeed,true);
    } else if (motif == "Candle") {
        candle(RGB[0], RGB[1], RGB[2],ledspeed,false);
    } else if (motif == "Etoile") {
        Etoile(RGB[0], RGB[1], RGB[2],ledspeed);
    } else if (motif == "Fire") {
        fire(RGB[0], RGB[1], RGB[2],ledspeed);
    #ifdef AUDIO
    } else if (motif == "Audio_color" && AUDIO) {
        color_sound(RGB[0], RGB[1], RGB[2],ledspeed,LED_volume_filter);
    } else if (motif == "Audio_laser" && AUDIO) {
        Laser_sound(strip.Color(RGB[0], RGB[1], RGB[2]),ledspeed,LED_volume_filter);
    }else if (motif == "Audio_K2000" && AUDIO) {
        K2000_sound(strip.Color(RGB[0], RGB[1], RGB[2]),ledspeed,LED_volume_filter,false);
    }else if (motif == "Audio_K2000_variation" && AUDIO) {
        K2000_variation_sound(ledspeed,LED_volume_filter,true);
    #endif
    } else {
        noir();
    }
  }
  
  void setpixelcolor(int i, uint32_t color) {
    strip.setPixelColor(i, color);
    if (SYMETRICAL == 2) {
       strip.setPixelColor(LED_COUNT-(i+1), color);
    }
  }
#endif
