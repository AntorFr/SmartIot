#ifdef LED
  // Fill the dots one after the other with a color
  void Wipe(uint32_t c, uint8_t wait) {
    static uint16_t y = 0;
    static boolean sens = true;
    uint32_t color_i;
    
    unsigned long currentMillis = millis();
    
    if(currentMillis - timeLED > wait * 1.5 * Led_density ) {
        for(uint16_t i=0; i<Led_lenght; i++) {
          if (i>y) {
            color_i = strip.Color(0,0,0);
          } else {
            color_i = c;
          }
          if (sens) {
            setpixelcolor(i, color_i);
          } 
          else {
            setpixelcolor(Led_lenght-(i+1), color_i);
          }
        }
        strip.show();
        y++;
        if (y > (Led_lenght)) {
          //Serial.println("Changement de sens."); 
          sens = !sens;
          y=0;
        }
        timeLED = currentMillis;
    }
  }
  
  void Laser(uint32_t c, uint8_t wait) {
    static uint16_t y = 0;
    static boolean sens = true;
    uint32_t color_i;
    
    unsigned long currentMillis = millis();
    
    if(currentMillis - timeLED > wait * Led_density) {
        for(uint16_t i=0; i<Led_lenght; i++) {
          if (i>=y & sens) {
            color_i = strip.Color(0,0,0);
          } else if (i<=y & !sens) {
            color_i = strip.Color(0,0,0);
          } else {
            color_i = c;
          }
          setpixelcolor(i, color_i);
  
        }
        strip.show();
        y++;
        if (y > (Led_lenght)) {
          //Serial.println("Changement de sens."); 
          sens = !sens;
          y=0;
        }
        timeLED = currentMillis;
    }
  }
  
  
  void noel( uint8_t wait) {
    static int y = 0;
    unsigned long currentMillis = millis();
    
    if(currentMillis - timeLED > ledspeed * 5 * Led_density ) {
      for (uint16_t i=0; i < Led_lenght; i++) {
        if(int((i+y)/5)%6==0) {
          setpixelcolor(i, strip.Color(255,0,0));    //turn every third pixel on
        } else if (int((i+y)/5)%6==3)  {
          setpixelcolor(i, strip.Color(0,255,0));    //turn every third pixel on
        } else {
          setpixelcolor(i, strip.Color(0,0,0));
        }
      }
      strip.show();
      y++;
      if (y > (5*6)) {
          y=0;
        }
      timeLED = currentMillis;
    }
  
  }
  
  void Clignote(uint32_t c, uint8_t wait) {
    static int q = 0;
    unsigned long currentMillis = millis();
    
    if(currentMillis - timeLED > ledspeed * 5) {
      for (uint16_t i=0; i < Led_lenght; i++) {
        if((int(i/10)+q)%3==0) {
          setpixelcolor(i, c);    //turn every third pixel on
        } else {
          setpixelcolor(i, 0);    //turn every third pixel on
        }
      }
      strip.show();
      q=(q+1)%3;
      timeLED = currentMillis;
    }
  
  }
  
  void fire(int Initred, int Initgreen, int Initblue, uint8_t wait) {
    static int q = 0;
    unsigned long currentMillis = millis();
  
    if(currentMillis - timeLED > ledspeed * random(3,8) * Led_density ) {
      for (uint16_t i=0; i < LED_COUNT; i++) {
        int flicker = random(0,150);
        int r = max(Initred-flicker,0);
        int g = max(Initgreen-flicker,0);
        int b = max(Initblue-flicker,0);
        strip.setPixelColor(i,strip.Color(r,g, b));
      }
      strip.show();
      timeLED = currentMillis;
    }
  
  }
  
  
  void police(uint8_t wait) {
    static int q = 0;
    unsigned long currentMillis = millis();
    
    if(currentMillis - timeLED > ledspeed * 7) {
      for (uint16_t i=0; i < Led_lenght; i++) {
        if((int(i/10+q))%4==0) {
          setpixelcolor(i, strip.Color(255,0,0));    //turn every third pixel on
        } else if((int(i/10+q))%4==2) {
          setpixelcolor(i, strip.Color(0,0,255));    //turn every third pixel on
        } else {
          setpixelcolor(i, 0);    //turn every third pixel on
        }
      }
      strip.show();
      q=(q+1)%2;
      timeLED = currentMillis;
    }
  
  }
  
  void candle(int Initred, int Initgreen, int Initblue, int ledspeed, boolean lightning) {
    // Update the colors.
    static unsigned long y = 0;
    static boolean sens = true;
    static int r = 0;
    byte red,green,blue;
    
    unsigned long currentMillis = millis();
    if(currentMillis - timeLED > ledspeed * 2 ) {    
      for(byte i = 0; i < LED_COUNT; i++)
      {
        
         float intensit = -1*pow((int(r+y+i*2*pow(-1,int(i/10)))%120)/40.1,2);
         red = Initred*exp(intensit);
         green = Initgreen *exp(intensit);
         blue = Initblue *exp(intensit);
         
         if (lightning == true && ((y== 18)%(60+r) ||(y== 18+1)%(60+r) || y== (18+4)%(60+r))) {
           red = 0;
           green = 0;
           blue = 0;
         }
         
        strip.setPixelColor(i, strip.Color(red, green, blue));
      }
      // Write the colors to the LED strip.
      strip.show(); 
      
      timeLED = currentMillis;
      
      if (sens) y++;
      else y--;
      
      if (y==60) r=random(20);
      
      if(y>=120) sens=false;
      if(y<=0) sens =true;
    }
  }
  
  
  //Theatre-style crawling lights with rainbow effect
  void theaterChaseRainbow(uint8_t wait) {
    for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
      for (int q=0; q < 3; q++) {
        for (uint16_t i=0; i < Led_lenght; i=i+3) {
          setpixelcolor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
  
        delay(wait);
  
        for (uint16_t i=0; i < Led_lenght; i=i+3) {
          setpixelcolor(i+q, 0);        //turn every third pixel off
        }
      }
    }
  }
  
  
  
  void color(uint32_t c) {
    // Update the colors.
  
    unsigned long currentMillis = millis();
    
    if(currentMillis - timeLED > 5000) {
  
      for(int i = 0; i < Led_lenght; i++)
      {
        setpixelcolor(i, c);
      }
      strip.show();
      
      timeLED = currentMillis;
    }
  }
  
  
  void noir() {
    // Update the colors.
  
    unsigned long currentMillis = millis();
    if(currentMillis - timeLED > 5000) {
  
      for(byte i = 0; i < Led_lenght; i++)
      {
        setpixelcolor(i, strip.Color(0, 0, 0));
      }
      strip.show(); 
  
      timeLED = currentMillis;
    }
  }
  
  void Blink(int Initred, int Initgreen, int Initblue, int ledspeed) {
    // Update the colors.
    static unsigned long y = 0;
    static boolean sens = true;
    byte red,green,blue;
    
    unsigned long currentMillis = millis();
    if(currentMillis - timeLED > ledspeed ) {
      
      float intensit = -1*pow(y/40.1,2);
      red = Initred*exp(intensit);
      green = Initgreen*exp(intensit);
      blue = Initblue*exp(intensit);
      
      for(byte i = 0; i < Led_lenght; i++)
      {
        setpixelcolor(i, strip.Color(red, green, blue));
      }
      // Write the colors to the LED strip.
      strip.show(); 
      
      timeLED = currentMillis;
      
      if (sens) y++;
      else y--;
      
      if(y>=120) sens=false;
      if(y<=0) sens =true;
    }
  }
  
  void StarTrek(int Initred, int Initgreen, int Initblue) {
    // Update the colors.
    static int y = 0;
    
    unsigned long currentMillis = millis();
    if(currentMillis - timeLED > ledspeed * 1.5 * Led_density) {
  
      for(int i = 0; i < int(Led_lenght/2); i++)
      {
  
        byte red,green,blue;
        if(i > y) {
          red = 0;
          green = 0;
          blue = 0;
        } 
        else {
          red = int(Initred/pow(1+(y-i)/4,3));
          green = int(Initgreen/pow(1+(y-i)/4,3));
          blue = int(Initblue/pow(1+(y-i)/4,3));
        }
        setpixelcolor(i, strip.Color(red, green, blue));
        setpixelcolor((Led_lenght-(i+1)), strip.Color(red, green, blue));
      }
      if (Led_lenght%2>0){
        //setpixelcolor(int(Led_lenght/2), strip.Color(Initred, Initgreen, Initblue));
        setpixelcolor(int(Led_lenght/2), strip.Color(0, 0, 0));
      }
      // Write the colors to the LED strip.
      y++;
  
      if (y == (Led_lenght/2)) {
        y=0;
      }
      strip.show();
      timeLED = currentMillis;
    }
  }
  
  
  
  void K2000(int Initred, int Initgreen, int Initblue) {
    // Update the colors.
    static boolean sens = true;
    static uint16_t y = 0;
  
    unsigned long currentMillis = millis();
    if(currentMillis - timeLED > ledspeed * Led_density) {
  
      for(byte i = 0; i < Led_lenght; i++)
      {
        byte red,green,blue;
        if(i > y) {
          red = 0;
          green = 0;
          blue = 0;
        } 
        else {
          red = int(Initred/pow(1+(y-i)/4,3));
          green = int(Initgreen/pow(1+(y-i)/4,3));
          blue = int(Initblue/pow(1+(y-i)/4,3));
        }
        if (sens) {
          setpixelcolor(i, strip.Color(red, green, blue));
        } 
        else {
          setpixelcolor(Led_lenght-i, strip.Color(red, green, blue));
        }
  
      }
      // Write the colors to the LED strip.
      strip.show();  
  
      y++;
  
      //Serial.println(y); 
      if (y == (Led_lenght)) {
        //Serial.println("Changement de sens."); 
        sens = !sens;
        y=0;
      }
      timeLED = currentMillis;
    }
  }
  
  void Variation(int ledspeed) {
    uint16_t i;
    static uint16_t j = 0;
    
    unsigned long currentMillis = millis();
    
    if(currentMillis - timeLED > ledspeed * Led_density) {
      for(i=0; i< LED_COUNT; i++) {
        strip.setPixelColor(i, Wheel((i+j) & 255));
      }
      strip.show();
      timeLED = currentMillis;
      j=(j+1)%256;
     }
  
  }
  
  void all_off() {
    for(int i = 0; i < Led_lenght; i++)
    {
      setpixelcolor(i, 0);
    }
    strip.show();
  }
  
  void all_color(uint32_t c) {
    for(int i = 0; i < Led_lenght; i++)
    {
      setpixelcolor(i, c);
    }
    strip.show();
  }
  
  
  void Etoile(int Initred, int Initgreen, int Initblue, int ledspeed) {
    uint16_t i;
    static int etoiles[LED_COUNT] = {0}; // Led_lenght
    unsigned long currentMillis = millis();
    
    if(currentMillis - timeLED > 30) {
      for(i=0; i< LED_COUNT; i++) {
        if(etoiles[i] <= 0) {
          if (random(LED_DENSITY*ledspeed)==0) { //
             etoiles[i] = 255 ;
          }
        }
        //if (i==0){Serial.println(etoiles[i]);}
        if (etoiles[i] > 0) {
          float intensit = -1*pow(etoiles[i]/80.1,2);
          strip.setPixelColor(i, strip.Color(
            Initred*exp(intensit),
            Initgreen*exp(intensit), 
            Initblue*exp(intensit)
          ));
          if (etoiles[i]%2 == 1) {
            etoiles[i] = etoiles[i]-2;
            if(etoiles[i] <= 0) {etoiles[i]=2;}
          } else {
            etoiles[i]= etoiles[i]+2;
            if(etoiles[i]>240) {etoiles[i]=0;} 
          }
        }
      }
      strip.show();
      timeLED = currentMillis;
     }
  
  }
  
  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
      return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if(WheelPos < 170) {
      WheelPos -= 85;
      return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
#endif
