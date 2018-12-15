#if defined (LED) && defined (LED_AUDIO)
void color_sound(int Initred, int Initgreen, int Initblue,int ledspeed, float audio) {
  int red,green,blue;
  
  unsigned long currentMillis = millis();
  if(currentMillis - timeLED > ledspeed * 2) {
    
     volume_moy = _max((audio * (100 - 99) + (volume_moy * 99)),100)/100;
     int level = _min(int(audio*25/volume_moy),255);
     
     red = int(Initred*level/255);
     green = int(Initgreen *level/255);
     blue = int(Initblue *level/255);
      
     for(int i = 0; i < Led_lenght; i++)
     {
        setpixelcolor(i, strip.Color(red, green, blue));
      }
      strip.show();
      timeLED = currentMillis;
    }
}

void Laser_sound(uint32_t c, int ledspeed, float audio) {  
  unsigned long currentMillis = millis();
  if(currentMillis - timeLED > ledspeed * 2) {
    
     volume_moy = _max((audio * (100 - 99) + (volume_moy * 99)),100)/100;
     int level = _min(int(audio*125/volume_moy),255);
     
     for(uint16_t i=0; i<Led_lenght; i++) {
      if (i<= int(level*Led_lenght/255)) {
          setpixelcolor(i, c);
      } else  {
          setpixelcolor(i,strip.Color(0,0,0));
      } 
    }
    strip.show();
    timeLED = currentMillis;
  }
}

void K2000_sound(uint32_t c, int ledspeed, float audio,boolean reverse) {  
  unsigned long currentMillis = millis();
  if(currentMillis - timeLED >  ledspeed * 2) {
    
     volume_moy = _max((audio * (100 - 99.5) + (volume_moy * 99.5)),100)/100;
     int level = _min(int(audio*32 /volume_moy),255);
     
     for(uint16_t i=0; i<Led_lenght/2; i++) {
      int y = (reverse?Led_lenght/2+i:i);
      if (i<= int(level*Led_lenght/255)) {
          setpixelcolor(y, c);
          setpixelcolor(Led_lenght-(y+1), c);
      } else  {
          setpixelcolor(y,strip.Color(0,0,0));
          setpixelcolor(Led_lenght-(y+1), strip.Color(0,0,0));
      } 

    }
    strip.show();
    timeLED = currentMillis;
  }
}

void K2000_variation_sound(int ledspeed, float audio,boolean reverse) {
  // Update the colors.
  static uint16_t j = 0;
  const uint32_t n = strip.Color(0,0,0);
  unsigned long currentMillis = millis();
  
  if(currentMillis - timeLED > 100  ) {
     volume_moy = _max((audio * (100 - 99.8) + (volume_moy * 99.8)),100)/100;
     int level = _min(int(audio*32 /volume_moy),255);
     
     /*
     Serial.print(audio);
     Serial.print(" / ");
     Serial.print(volume_moy);
     Serial.print(" = ");
     Serial.println(level);
     */
     
     for(uint16_t i=0; i<Led_lenght/2; i++) {
      int y = (reverse?Led_lenght/2+i:i);
       
      if (i<= int(level*Led_lenght/255)) {
          setpixelcolor(y, Wheel((y+int(j)) & 255));
          setpixelcolor(Led_lenght-(y+1), Wheel((Led_lenght-(y+1)+int(j)) & 255));
      } else  {
          setpixelcolor(y,n);
          setpixelcolor(Led_lenght-(y+1), n);
      } 

    }
    strip.show();
    timeLED = currentMillis;
    j=(j+1)%256;
  }
}
#endif
