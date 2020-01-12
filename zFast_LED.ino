#if defined (LED) && defined (Fast_LED)

  #define FASTLED_ALLOW_INTERRUPTS 0

  #include <FastLED.h>
  #include <EEPROM.h>

  #define NUM_LEDS 200 //62 //lune 31 // stars 5 // Sun 26
  #define DATA_PIN D1
  CRGBArray<NUM_LEDS> leds;

  #define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

  #include "zFL_GradientPalettes.h";
  #include "zFL_Field.h"

  #define MILLI_AMPS         4000     // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
  #define FRAMES_PER_SECOND  60 // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.

  uint8_t patternIndex = 0;

  const uint8_t brightnessCount = 5;
  uint8_t brightnessMap[brightnessCount] = { 16, 32, 64, 128, 255 };
  uint8_t brightnessIndex = 0;
  
  // ten seconds per color palette makes a good demo
  // 20-120 is better for deployment
  uint8_t secondsPerPalette = 10;
  
  // COOLING: How much does the air cool as it rises?
  // Less cooling = taller flames.  More cooling = shorter flames.
  // Default 50, suggested range 20-100
  uint8_t cooling = 49;
  
  // SPARKING: What chance (out of 255) is there that a new spark will be lit?
  // Higher chance = more roaring fire.  Lower chance = more flickery fire.
  // Default 120, suggested range 50-200.
  uint8_t sparking = 60;
  uint8_t speed = 30;
  
  ///////////////////////////////////////////////////////////////////////
  
  // Forward declarations of an array of cpt-city gradient palettes, and
  // a count of how many there are.  The actual color palette definitions
  // are at the bottom of this file.
  extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
  
  uint8_t gCurrentPaletteNumber = 0;
  
  CRGBPalette16 gCurrentPalette( CRGB::Black);
  CRGBPalette16 gTargetPalette( gGradientPalettes[0] );
  
  CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
  
  uint8_t currentPatternIndex = 0; // Index number of which pattern is current
  uint8_t autoplay = 0;
  
  uint8_t autoplayDuration = 30;
  unsigned long autoPlayTimeout = 0;
  
  uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
  CRGB solidColor = CRGB::Blue;

  void setSolidColor(CRGB color);
  void setSolidColor(uint8_t r, uint8_t g, uint8_t b);
  void heatMap(CRGBPalette16 palette, bool up);
  void heatMap(led_obj obj,CRGBPalette16 palette, bool up);
  void addGlitter(led_obj obj, uint8_t chanceOfGlitter);
  void colorwaves( CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette);
  void palettetest( CRGB* ledarray, uint16_t numleds, const CRGBPalette16& gCurrentPalette);


    // scale the brightness of all pixels down
  void dimAll(led_obj obj,byte value)
  {
    for (int i = obj.start_point; i < obj.start_point+obj.nb_led; i++) {
      leds[i].nscale8(value);
    }
  }
  
  // scale the brightness of all pixels down
  void dimAll(byte value)
  {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].nscale8(value);
    }
  }
  
  typedef void (*Pattern)();
  typedef Pattern PatternList[];
  typedef struct {
    Pattern pattern;
    String name;
  } PatternAndName;
  typedef PatternAndName PatternAndNameList[];
  
  #include "zFL_Twinkles.h"
  #include "zFL_TwinkleFOX.h"
  // List of patterns to cycle through.  Each is defined as a separate function below.
  
  PatternAndNameList patterns = {
  
    { pride,                  "Pride" },
    { colorWaves,             "Color Waves" },
  
    // twinkle patterns
    { rainbowTwinkles,        "Rainbow Twinkles" },
    { snowTwinkles,           "Snow Twinkles" },
    { cloudTwinkles,          "Cloud Twinkles" },
    { incandescentTwinkles,   "Incandescent Twinkles" },
  
    // TwinkleFOX patterns
    { retroC9Twinkles,        "Retro C9 Twinkles" },
    { redWhiteTwinkles,       "Red & White Twinkles" },
    { blueWhiteTwinkles,      "Blue & White Twinkles" },
    { redGreenWhiteTwinkles,  "Red, Green & White Twinkles" },
    { fairyLightTwinkles,     "Fairy Light Twinkles" },
    { snow2Twinkles,          "Snow 2 Twinkles" },
    { hollyTwinkles,          "Holly Twinkles" },
    { iceTwinkles,            "Ice Twinkles" },
    { partyTwinkles,          "Party Twinkles" },
    { forestTwinkles,         "Forest Twinkles" },
    { lavaTwinkles,           "Lava Twinkles" },
    { fireTwinkles,           "Fire Twinkles" },
    { cloud2Twinkles,         "Cloud 2 Twinkles" },
    { oceanTwinkles,          "Ocean Twinkles" },
  
    { rainbow,                "Rainbow" },
    { rainbowWithGlitter,     "Rainbow With Glitter" },
    { rainbowSolid,           "Solid Rainbow" },
    { confetti,               "Confetti" },
    { sinelon,                "Sinelon" },
    { bpm,                    "Beat" },
    { juggle,                 "Juggle" },
    { fire,                   "Fire" },
    { water,                  "Water" },
  
    { showSolidColor,         "Solid Color" }
  };
  
  const uint8_t patternCount = ARRAY_SIZE(patterns);

  #include "zFL_Fields.h"

  void setupLED()
  {
    brightness = 255;
  
    FastLED.addLeds<WS2812,DATA_PIN, RGB>(leds, NUM_LEDS); //GRB RGB //<NEOPIXEL,DATA_PIN, GRB>
    FastLED.setDither(false);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(brightness);
    //  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
  
    EEPROM.begin(512);

    FastLED.setBrightness(255);
    autoPlayTimeout = millis() + (autoplayDuration * 1000);
    setAutoplay(1);
    setAutoplayDuration(10);
    
    loadSettings();
  
    trc( F("Heap: ") ); 
    trc(system_get_free_heap_size());
    trc( F("Boot Vers: ") ); 
    trc(system_get_boot_version());
    trc( F("CPU: ") ); 
    trc(system_get_cpu_freq());
    trc( F("SDK: ") ); 
    trc(system_get_sdk_version());
    trc( F("Chip ID: ") ); 
    trc(system_get_chip_id());
    trc( F("Flash ID: ") ); 
    trc(spi_flash_get_id());
    trc( F("Flash Size: ") ); 
    trc(ESP.getFlashChipRealSize());
    trc( F("Vcc: ") ); 
    trc(ESP.getVcc());
  


    //First write 
    //setPower(power);
    //setBrightness(brightness);
    //EEPROM.write(2, r);
    //EEPROM.write(3, g);
    //EEPROM.write(4, b);



    
    
  }

  void display_led() {
    // Add entropy to random number generator; we use a lot of it.
    random16_add_entropy(random(65535));
    
    if (power == 0) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      // FastLED.delay(15);
      return;
    }
  
    // EVERY_N_SECONDS(10) {
    //   Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
    // }
  
    // change to a new cpt-city gradient palette
    EVERY_N_SECONDS( secondsPerPalette ) {
      gCurrentPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
      gTargetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
  
      //    paletteIndex = addmod8( paletteIndex, 1, paletteCount);
      //    targetPalette = palettes[paletteIndex];
    }
  
    EVERY_N_MILLISECONDS(40) {
      // slowly blend the current palette to the next
      nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 8);
      //    nblendPaletteTowardPalette(currentPalette, targetPalette, 16);
      gHue++;  // slowly cycle the "base color" through the rainbow
    }
  
    if (autoplay && (millis() > autoPlayTimeout)) {
      trc(F("LED AdjustPattern"));
      adjustPattern(true);

      trc(F("LED patterns"));
      trc(patterns[currentPatternIndex].name);
      
      autoPlayTimeout = millis() + (autoplayDuration * 1000);
    }
  
    //Call the current pattern function once, updating the 'leds' array
    patterns[currentPatternIndex].pattern();
    FastLED.show();
  
    //insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);
  }


  void loadSettings()
  {
    brightness = EEPROM.read(0);
  
    currentPatternIndex = EEPROM.read(1);
    if (currentPatternIndex < 0)
      currentPatternIndex = 0;
    else if (currentPatternIndex >= patternCount)
      currentPatternIndex = patternCount - 1;
  
    byte r = EEPROM.read(2);
    byte g = EEPROM.read(3);
    byte b = EEPROM.read(4);
  
    if (r == 0 && g == 0 && b == 0)
    {
    }
    else
    {
      solidColor = CRGB(r, g, b);
    }
  
    power = EEPROM.read(5);
  
    autoplay = EEPROM.read(6);
    autoplayDuration = EEPROM.read(7);
  }
  
  void setPower(uint8_t value)
  {
    power = value == 0 ? 0 : 1;
  
    EEPROM.write(5, power);
    EEPROM.commit();
  
    //broadcastInt("power", power);
  }
  
  void setAutoplay(uint8_t value)
  {
    autoplay = value == 0 ? 0 : 1;
  
    EEPROM.write(6, autoplay);
    EEPROM.commit();
  
    //broadcastInt("autoplay", autoplay);
  }
  
  void setAutoplayDuration(uint8_t value)
  {
    autoplayDuration = value;
  
    EEPROM.write(7, autoplayDuration);
    EEPROM.commit();
  
    autoPlayTimeout = millis() + (autoplayDuration * 1000);
  
    //broadcastInt("autoplayDuration", autoplayDuration);
  }
  
  void setSolidColor(CRGB color)
  {
    setSolidColor(color.r, color.g, color.b);
  }
  
  void setSolidColor(uint8_t r, uint8_t g, uint8_t b)
  {
    solidColor = CRGB(r, g, b);
  
    EEPROM.write(2, r);
    EEPROM.write(3, g);
    EEPROM.write(4, b);
    EEPROM.commit();
  
    setPattern(patternCount - 1);
  
    //broadcastString("color", String(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b));
  }
  
  // increase or decrease the current pattern number, and wrap around at the ends
  void adjustPattern(bool up)
  {
    if (up)
      currentPatternIndex++;
    else
      currentPatternIndex--;
  
    // wrap around at the ends
    if (currentPatternIndex < 0)
      currentPatternIndex = patternCount - 1;
    if (currentPatternIndex >= patternCount)
      currentPatternIndex = 0;
  
    if (autoplay == 0) {
      EEPROM.write(1, currentPatternIndex);
      EEPROM.commit();
    }
  
    //broadcastInt("pattern", currentPatternIndex);
  }
  
  void setPattern(uint8_t value)
  {
    if (value >= patternCount)
      value = patternCount - 1;
  
    currentPatternIndex = value;
  
    if (autoplay == 0) {
      EEPROM.write(1, currentPatternIndex);
      EEPROM.commit();
    }
  
    //broadcastInt("pattern", currentPatternIndex);
  }
  
  void setPatternName(String name)
  {
    for (uint8_t i = 0; i < patternCount; i++) {
      if (patterns[i].name == name) {
        setPattern(i);
        break;
      }
    }
  }
  
  void adjustBrightness(bool up)
  {
    if (up && brightnessIndex < brightnessCount - 1)
      brightnessIndex++;
    else if (!up && brightnessIndex > 0)
      brightnessIndex--;
  
    brightness = brightnessMap[brightnessIndex];
  
    FastLED.setBrightness(brightness);
  
    EEPROM.write(0, brightness);
    EEPROM.commit();
  
    //broadcastInt("brightness", brightness);
  }
  
  void setBrightness(uint8_t value)
  {
    if (value > 255)
      value = 255;
    else if (value < 0) value = 0;
  
    brightness = value;
  
    FastLED.setBrightness(brightness);
  
    EEPROM.write(0, brightness);
    EEPROM.commit();
  
    //broadcastInt("brightness", brightness);
  }

  void MQTTtoLED(char * topicOri, JsonObject& LEDData){
      String topic = tolower(topicOri);
      if (topic == tolower(subjectMQTTtoLED) || topic == tolower(subjectMQTTtoAllLED)){
        trc(F("New LED commande received"));
        
        if(LEDData.containsKey("power")) { 
          setPower(LEDData["power"]);
        }
        if(LEDData.containsKey("brightness")) { 
          setBrightness(LEDData["brightness"]); 
        }
        if(LEDData.containsKey("autoplay")) {
            setAutoplay(LEDData["autoplay"]);
        }
        if(LEDData.containsKey("autoplay_duration")) {
            setAutoplayDuration(LEDData["autoplay_duration"]);
        }
        if(LEDData.containsKey("pattern")) {
            setPower(1);
            setAutoplay(0);
            setPatternName(LEDData["pattern"]); 
        }
        if(LEDData.containsKey("RGB")) {
            setPower(1);
            setAutoplay(0);
            setSolidColor(LEDData["RGB"][0],LEDData["RGB"][1],LEDData["RGB"][2]);
        }
        /*
        timezone =  LEDData["timezone"];
        setTimezone(timezone);

        
        flipClock =  LEDData["flipClock"];
        setFlipClock(flipClock);
        */
        if(LEDData.containsKey("list_patern")) {
          String patternNames = "[";
          for(uint8_t i = 0; i < patternCount; i++)
          {
            patternNames.concat("\"");
            patternNames.concat(patterns[i].name);
            patternNames.concat("\"");
            if(i < patternCount - 1)
            patternNames.concat(",");
          }
          patternNames.concat("]");

          //const char* c_patternNames = LEDData["patternNames"];
          //patternNames = String(c_patternNames);
          LEDData["patternNames"] = patternNames;
        }
        
        trc(F("LED configured"));
        pub(tolower(subjecLEDtoMQTT), LEDData);
      }
  }
 /*
  void strandTest()
  {
    static uint8_t i = 0;
  
    EVERY_N_SECONDS(1)
    {
      i++;
      if (i >= NUM_LEDS)
        i = 0;
    }
  
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  
    leds[i] = solidColor;
  }
  */
  void strandTest(led_obj obj)
  {
    static uint8_t i = obj.start_point;
  
    EVERY_N_SECONDS(1)
    {
      i++;
      if (i >= obj.nb_led)
        i = obj.start_point;
    }
  
    fill_solid(leds+obj.start_point, obj.nb_led, CRGB::Black);
  
    leds[i] = solidColor;
  }
  void strandTest() { strandTest((led_obj) {0,NUM_LEDS}); }
  
  void showSolidColor(led_obj obj)
  {
    fill_solid(leds+obj.start_point, obj.nb_led, solidColor);
  }
  void showSolidColor() { showSolidColor((led_obj) {0,NUM_LEDS});}
  
  // Patterns from FastLED example DemoReel100: https://github.com/FastLED/FastLED/blob/master/examples/DemoReel100/DemoReel100.ino
  void rainbow(led_obj obj)
  {
    // FastLED's built-in rainbow generator
    fill_rainbow(leds+obj.start_point, obj.nb_led, gHue, 255 / NUM_LEDS);
  }
  void rainbow() { rainbow((led_obj) {0,NUM_LEDS});  }
  
  void rainbowWithGlitter(led_obj obj)
  {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow(obj);
    addGlitter(obj,80);
  }
  void rainbowWithGlitter() { rainbowWithGlitter((led_obj) {0,NUM_LEDS});}

  
  void rainbowSolid(led_obj obj)
  {
    fill_solid(leds+obj.start_point, obj.nb_led, CHSV(gHue, 255, 255));
  }
  void rainbowSolid() {rainbowSolid((led_obj) {0,NUM_LEDS});}
  
  void confetti(led_obj obj)
  {
    fadeToBlackBy(leds+obj.start_point, obj.nb_led, 10);
  
    // random colored speckles that blink in and fade smoothly
    int pos = obj.start_point+random16(obj.nb_led);
    leds[pos] += CHSV( gHue + random8(64), 200, 255);
  }
  void confetti() { confetti((led_obj) {0,NUM_LEDS});  }
  
  void sinelon(led_obj obj)
  {
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds+obj.start_point, obj.nb_led, 20);
    int pos = beatsin16(speed, obj.start_point, obj.nb_led - 1);
    static int prevpos = 0;
    if ( pos < prevpos ) {
      fill_solid( leds + pos, (prevpos - pos) + 1, CHSV(gHue, 220, 255));
    } else {
      fill_solid( leds + prevpos, (pos - prevpos) + 1, CHSV( gHue, 220, 255));
    }
    prevpos = pos;
  }
  void sinelon() {sinelon((led_obj) {0,NUM_LEDS});}
  
  void bpm(led_obj obj)
  {
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t beat = beatsin8( speed, 64, 255);
    for ( int i = obj.start_point; i < obj.nb_led; i++) {
      leds[i] = ColorFromPalette(gCurrentPalette, gHue + (i * 2), beat - gHue + (i * 10));
    }
  }
  void bpm() {bpm((led_obj) {0,NUM_LEDS});}

  
  void juggle(led_obj obj)
  {
    static uint8_t    numdots =   4; // Number of dots in use.
    static uint8_t   faderate =   2; // How long should the trails be. Very low value = longer trails.
    static uint8_t     hueinc =  255 / numdots - 1; // Incremental change in hue between each dot.
    static uint8_t    thishue =   0; // Starting hue.
    static uint8_t     curhue =   0; // The current hue
    static uint8_t    thissat = 255; // Saturation of the colour.
    static uint8_t thisbright = 255; // How bright should the LED/display be.
    static uint8_t   basebeat =   5; // Higher = faster movement.
  
    static uint8_t lastSecond =  99;  // Static variable, means it's only defined once. This is our 'debounce' variable.
    uint8_t secondHand = (millis() / 1000) % 30; // IMPORTANT!!! Change '30' to a different value to change duration of the loop.
  
    if (lastSecond != secondHand) { // Debounce to make sure we're not repeating an assignment.
      lastSecond = secondHand;
      switch (secondHand) {
        case  0: numdots = 1; basebeat = 20; hueinc = 16; faderate = 2; thishue = 0; break; // You can change values here, one at a time , or altogether.
        case 10: numdots = 4; basebeat = 10; hueinc = 16; faderate = 8; thishue = 128; break;
        case 20: numdots = 8; basebeat =  3; hueinc =  0; faderate = 8; thishue = random8(); break; // Only gets called once, and not continuously for the next several seconds. Therefore, no rainbows.
        case 30: break;
      }
    }
  
    // Several colored dots, weaving in and out of sync with each other
    curhue = thishue; // Reset the hue values.
    fadeToBlackBy(leds+obj.start_point, obj.nb_led, faderate);
    for ( int i = 0; i < numdots; i++) {
      //beat16 is a FastLED 3.1 function
      leds[beatsin16(basebeat + i + numdots, obj.start_point, obj.nb_led)] += CHSV(gHue + curhue, thissat, thisbright);
      curhue += hueinc;
    }
  }
  void juggle() {juggle((led_obj) {0,NUM_LEDS});}
  
  void fire(led_obj obj)
  {
    heatMap(obj,HeatColors_p, true);
  }
  void fire(){fire((led_obj) {0,NUM_LEDS});}
  
  void water(led_obj obj)
  {
    heatMap(obj,IceColors_p, false);
  }
  void water(){water((led_obj) {0,NUM_LEDS});}
  
  // Pride2015 by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
  // This function draws rainbows with an ever-changing,
  // widely-varying set of parameters.
  void pride(led_obj obj)
  {
    static uint16_t sPseudotime = 0;
    static uint16_t sLastMillis = 0;
    static uint16_t sHue16 = 0;
  
    uint8_t sat8 = beatsin88( 87, 220, 250);
    uint8_t brightdepth = beatsin88( 341, 96, 224);
    uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
    uint8_t msmultiplier = beatsin88(147, 23, 60);
  
    uint16_t hue16 = sHue16;//gHue * 256;
    uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
    uint16_t ms = millis();
    uint16_t deltams = ms - sLastMillis ;
    sLastMillis  = ms;
    sPseudotime += deltams * msmultiplier;
    sHue16 += deltams * beatsin88( 400, 5, 9);
    uint16_t brightnesstheta16 = sPseudotime;
  
    for ( uint16_t i = obj.start_point ; i < obj.nb_led; i++) {
      hue16 += hueinc16;
      uint8_t hue8 = hue16 / 256;
  
      brightnesstheta16  += brightnessthetainc16;
      uint16_t b16 = sin16( brightnesstheta16  ) + 32768;
  
      uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
      uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
      bri8 += (255 - brightdepth);
  
      CRGB newcolor = CHSV( hue8, sat8, bri8);
  
      uint16_t pixelnumber = i;
      pixelnumber = (obj.nb_led - 1) - pixelnumber;
  
      nblend( leds[pixelnumber], newcolor, 64);
    }
  }
  void pride() {pride((led_obj) {0,NUM_LEDS});}
  
  void radialPaletteShift(led_obj obj)
  {
    for (uint8_t i = obj.start_point; i < obj.nb_led; i++) {
      // leds[i] = ColorFromPalette( gCurrentPalette, gHue + sin8(i*16), brightness);
      leds[i] = ColorFromPalette(gCurrentPalette, i + gHue, 255, LINEARBLEND);
    }
  }
  void radialPaletteShift() {radialPaletteShift((led_obj) {0,NUM_LEDS});}
  
  // based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
  void heatMap(led_obj obj,CRGBPalette16 palette, bool up)
  {
    fill_solid(leds+obj.start_point, obj.nb_led, CRGB::Black);
  
    // Add entropy to random number generator; we use a lot of it.
    random16_add_entropy(random(256));
  
    // Array of temperature readings at each simulation cell
    static const uint8_t halfLedCount = obj.nb_led / 2;
    static byte heat[2][NUM_LEDS/2];
  
    byte colorindex;
  
    for (uint8_t x = 0; x < 2; x++) {
      // Step 1.  Cool down every cell a little
      for ( int i = 0; i < halfLedCount; i++) {
        heat[x][i] = qsub8( heat[x][i],  random8(0, ((cooling * 10) / halfLedCount) + 2));
      }
  
      // Step 2.  Heat from each cell drifts 'up' and diffuses a little
      for ( int k = halfLedCount - 1; k >= 2; k--) {
        heat[x][k] = (heat[x][k - 1] + heat[x][k - 2] + heat[x][k - 2] ) / 3;
      }
  
      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      if ( random8() < sparking ) {
        int y = random8(7);
        heat[x][y] = qadd8( heat[x][y], random8(160, 255) );
      }
  
      // Step 4.  Map from heat cells to LED colors
      for ( int j = 0; j < obj.nb_led / 2; j++) {
        // Scale the heat value from 0-255 down to 0-240
        // for best results with color palettes.
        colorindex = scale8(heat[x][j], 240);
  
        CRGB color = ColorFromPalette(palette, colorindex);
  
        if (up) {
          if (x == 0) {
            leds[obj.start_point +(halfLedCount - 1) - j] = color;
          }
          else {
            leds[obj.start_point+ halfLedCount + j] = color;
          }
        }
        else {
          if (x == 0) {
            leds[obj.start_point+ j] = color;
          }
          else {
            leds[obj.start_point+ (obj.nb_led - 1) - j] = color;
          }
        }
      }
    }
  }
  void heatMap(CRGBPalette16 palette, bool up) {heatMap((led_obj) {0,NUM_LEDS},palette,up);}


  void addGlitter(led_obj obj, uint8_t chanceOfGlitter)
  {
    if ( random8() < chanceOfGlitter) {
      leds[obj.start_point+random16(obj.nb_led) ] += CRGB::White;
    }
  }
  void addGlitter( uint8_t chanceOfGlitter) {addGlitter((led_obj) {0,NUM_LEDS},chanceOfGlitter);}

  ///////////////////////////////////////////////////////////////////////
  
  // Forward declarations of an array of cpt-city gradient palettes, and
  // a count of how many there are.  The actual color palette definitions
  // are at the bottom of this file.
  extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
  extern const uint8_t gGradientPaletteCount;
  
  uint8_t beatsaw8( accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255,
                    uint32_t timebase = 0, uint8_t phase_offset = 0)
  {
    uint8_t beat = beat8( beats_per_minute, timebase);
    uint8_t beatsaw = beat + phase_offset;
    uint8_t rangewidth = highest - lowest;
    uint8_t scaledbeat = scale8( beatsaw, rangewidth);
    uint8_t result = lowest + scaledbeat;
    return result;
  }
  
  void colorWaves()
  {
    colorwaves( leds, NUM_LEDS, gCurrentPalette);
  }
  
  // ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
  // This function draws color waves with an ever-changing,
  // widely-varying set of parameters, using a color palette.
  void colorwaves( CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette)
  {
    static uint16_t sPseudotime = 0;
    static uint16_t sLastMillis = 0;
    static uint16_t sHue16 = 0;
  
    // uint8_t sat8 = beatsin88( 87, 220, 250);
    uint8_t brightdepth = beatsin88( 341, 96, 224);
    uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
    uint8_t msmultiplier = beatsin88(147, 23, 60);
  
    uint16_t hue16 = sHue16;//gHue * 256;
    uint16_t hueinc16 = beatsin88(113, 300, 1500);
  
    uint16_t ms = millis();
    uint16_t deltams = ms - sLastMillis ;
    sLastMillis  = ms;
    sPseudotime += deltams * msmultiplier;
    sHue16 += deltams * beatsin88( 400, 5, 9);
    uint16_t brightnesstheta16 = sPseudotime;
  
    for ( uint16_t i = 0 ; i < numleds; i++) {
      hue16 += hueinc16;
      uint8_t hue8 = hue16 / 256;
      uint16_t h16_128 = hue16 >> 7;
      if ( h16_128 & 0x100) {
        hue8 = 255 - (h16_128 >> 1);
      } else {
        hue8 = h16_128 >> 1;
      }
  
      brightnesstheta16  += brightnessthetainc16;
      uint16_t b16 = sin16( brightnesstheta16  ) + 32768;
  
      uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
      uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
      bri8 += (255 - brightdepth);
  
      uint8_t index = hue8;
      //index = triwave8( index);
      index = scale8( index, 240);
  
      CRGB newcolor = ColorFromPalette( palette, index, bri8);
  
      uint16_t pixelnumber = i;
      pixelnumber = (numleds - 1) - pixelnumber;
  
      nblend( ledarray[pixelnumber], newcolor, 128);
    }
  }
  
  // Alternate rendering function just scrolls the current palette
  // across the defined LED strip.
  void palettetest( CRGB* ledarray, uint16_t numleds, const CRGBPalette16& gCurrentPalette)
  {
    static uint8_t startindex = 0;
    startindex--;
    fill_palette( ledarray, numleds, startindex, (256 / NUM_LEDS) + 1, gCurrentPalette, 255, LINEARBLEND);
  }
 

/* 
  void display_led()
  {
    timeClient.update();
  
    if(power < 1) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      FastLED.delay(15);
      return;
    }
  
    uint8_t delay = patterns[patternIndex].drawFrame();
    trc(F("LED patterns"));
    trc(patterns[patternIndex].name);
  
    // send the 'leds' array out to the actual LED strip
    FastLED.show();
  
    // insert a delay to keep the framerate modest
    FastLED.delay(delay);
  
    // blend the current palette to the next
    EVERY_N_MILLISECONDS(40) {
      nblendPaletteTowardPalette(currentPalette, targetPalette, 16);
    }
  
    EVERY_N_MILLISECONDS( 40 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  
    // slowly change to a new palette
    EVERY_N_SECONDS(SECONDS_PER_PALETTE) {
        paletteIndex++;
        if (paletteIndex >= paletteCount) paletteIndex = 0;
        targetPalette = palettes[paletteIndex];
        trc(F("LED targetPalette"));
        trc(paletteIndex);
    };
    }

  
 void loadSettings()
{
  brightness = EEPROM.read(0);

  currentPatternIndex = EEPROM.read(1);
  if (currentPatternIndex < 0)
    currentPatternIndex = 0;
  else if (currentPatternIndex >= patternCount)
    currentPatternIndex = patternCount - 1;

  byte r = EEPROM.read(2);
  byte g = EEPROM.read(3);
  byte b = EEPROM.read(4);

  if (r == 0 && g == 0 && b == 0)
  {
  }
  else
  {
    solidColor = CRGB(r, g, b);
  }

  power = EEPROM.read(5);

  autoplay = EEPROM.read(6);
  autoplayDuration = EEPROM.read(7);
}

void setPower(uint8_t value)
{
  power = value == 0 ? 0 : 1;

  EEPROM.write(5, power);
  EEPROM.commit();

  broadcastInt("power", power);
}*/

#endif
