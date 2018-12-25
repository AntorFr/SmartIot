#ifdef LED

  #include <Adafruit_NeoPixel.h>
  
  const float Led_density = round( 144 * 100 / LED_DENSITY )/100;
  const int Led_lenght = LED_COUNT / SYMETRICAL;
  
  unsigned long timeLED = 0; // When the sensor was last read   

  Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, STRIPPIN, NEO_GRB + NEO_KHZ800); //NEO_GRB //NEO_RGB
  
  String motif = String(50);//LED status flag
  int ledspeed = 30;
  int RGB[3] = {0,0,0};
      
  #ifdef LED_AUDIO
    static float volume_filter = 1;
    int volume;
    static float volume_moy= 1;
    unsigned long timeLED_AUDIO = 0;
  #endif
  
  void setupLED(){
    #ifdef LED_AUDIO
      pinMode(AUDIO_AIN, INPUT);
      pinMode(AUDIO_DIN, OUTPUT);
    #endif

    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
  
    motif = "noir";
    all_off();
    
  }
  
  void MQTTtoLED(char * topicOri, JsonObject& LEDData){
      String topic = topicOri;
      if (topic == subjectMQTTtoLED || topic == subjectMQTTtoAllLED){
        trc(F("New LED commande received"));
        const char* sensor = LEDData["motif"];
        motif = String(sensor);
        ledspeed = int(LEDData["speed"]);
        RGB[0] = LEDData["RGB"][0];
        RGB[1] = LEDData["RGB"][1];
        RGB[2] = LEDData["RGB"][2];
        timeLED = 0;
        noir();
        timeLED = 0;
        trc(F("LED configured"));
        pub(subjecLEDtoMQTT, LEDData);
      }
  }

  #ifdef LED_AUDIO
  void read_audio(){
     unsigned long now = millis();
      if (now > (timeLED_AUDIO + 5)) {
        timeLED_AUDIO = now;
        volume = analogRead(AUDIO_AIN);
        volume_filter = (volume * (1 - LED_filterVal)) + (volume_filter * LED_filterVal);
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
    #ifdef LED_AUDIO
    } else if (motif == "Audio_color") {
        color_sound(RGB[0], RGB[1], RGB[2],ledspeed,volume_filter);
    } else if (motif == "Audio_laser") {
        Laser_sound(strip.Color(RGB[0], RGB[1], RGB[2]),ledspeed,volume_filter);
    }else if (motif == "Audio_K2000") {
        K2000_sound(strip.Color(RGB[0], RGB[1], RGB[2]),ledspeed,volume_filter,false);
    }else if (motif == "Audio_K2000_variation") {
        K2000_variation_sound(ledspeed,volume_filter,true);
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
