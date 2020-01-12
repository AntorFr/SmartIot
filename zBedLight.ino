#ifdef BedLight

#define Fast_LED "FL"
#define NTP "NTP"

const led_obj moon = { 0 , 31};
const led_obj stars = { 31 , 5};
const led_obj sun = { 36 , 26};

const led_obj objects[] = {moon,stars,sun};

typedef void (*BL_mode)();
typedef BL_mode BL_modeList[];
typedef struct {
  uinit8_t mode_id;
  BL_mode blmode;
  String name;
} BLModeAndName;
typedef BLModeAndName BLModeAndNameList[];

PatternAndNameList BL_modes = {
    { 1, Sunshine,       "Sunshine"},
    { 2, Wake_up,        "Wake up"},
    { 3, Speed_up,       "Speed_up"},
    { 4, Time_to_leave,  "Time to leave"},
    { 10, Go_to_bed,      "Going to bed" },
    { 11, Sleep_time,     "Sleep time" },
    { 12, Fairy_night,    "Fairy night" },
};

enum Daytype {
  weekdays,
  weekends,
  hollidays
};

typedef struct {
  uinit8_t mode_id;
  Daytype daytype;
  uinit16_t start_time;
  uinit8_t duration;
} BLModeIDConfig;

typedef BLModeIDConfig BLModeIDConfigs[];

BLModeIDConfigs BL_modesConfig = {
  {1,weekdays,7*60+15,30},
  {2,weekdays,7*60+45,15},
  {3,weekdays,8*60,15},
  {4,weekdays,8*60+15,15},
  {10,weekdays,20*60,30},
  {11,weekdays,20*60+30,30},
  {12,weekdays,21*60,30},

  {1,weekends,8*60+15,30},
  {2,weekends,8*60+45,30},
  {10,weekends,20*60+30,30},
  {11,weekends,21*60,30},
  {12,weekends,21*60+30,30}
}
  
const uint8_t BL_modeCount = ARRAY_SIZE(BL_modes);
uint8_t BLCurentModeIndex = 0;


void display_BedLight() {
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(65535));

  uinit8_t curentmodeid = 0
  uint16_t mins = (tm.tm_hour * 60 + tm.tm_min);
  
  
  if ((BL_modesConfig[BLCurentModeIndex].start_time > mins) && (BL_modesConfig[BLCurentModeIndex].start_time+BL_modesConfig[BLCurentModeIndex].duration <= mins)) {
     curentmodeid = BL_modesConfig[BLCurentModeIndex].mode_id;
  } else {
    for (uint8_t i = 0; i < BL_modeCount; i++) {
      if ((BL_modesConfig[i].start_time > mins) && (BL_modesConfig[i].start_time+BL_modesConfig[i].duration <= mins) ) {
        BLCurentModeIndex = i;
        mode_actif = true;
        curentmodeid = BL_modesConfig[i].mode_id;
        break;    
      } 
    }
  }

  if (curentmodeid == 0) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      return;
  }

 
  //Todo find blmode from curentmodeid
  BL_modesConfig[i].blmode();
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND); 
  return;
}

void Sunshine(){
  uint16_t nbsec = (uint32_t(tm.tm_hour) * 3600 + tm.tm_min * 60 + tm.tm_sec) - uint32_t(BLCurentMode.start_time) * 60);
  uint16_t pct = uint32_t(nbsec) * 255   / (uint32_t(BLCurentMode.duration) * 60);

  setBrightness(pct);
  setSolidColor(255, 255, 255);
  showSolidColor(sun);
}

void Wake_up(){
  setBrightness(255);
  oceanTwinkles(sun);
}

void Speed_up(){
  setBrightness(255);
  iceTwinkles(sun);
  addGlitter(sun,80);
}

void Time_to_leave(){
  setBrightness(255);
  sinelon(sun);
}

/*
* Bool : present / absent 
* 
* Jour d'ecole:
* de 20h a 20h30 : lune + etoile de couleur 
* de 20h30 a 21h00 : lune s'eteint progressivement + etoile sintilles avec intensité
* de 21h00 a 21h30 : lune + etoile etoile avec intensité
* de 7h15 a 7h45 : Soleil : progressive bright
* de 7h45 a 8h00 : Soleil : full bright / Meto
* de 8h00 a 8h15 : Soleil meteo cyclic
* 8h15 a 8h30 : Soleil meteo clinote
* 
* 
* Jour sans ecole:
* meme chose mais pans aux meme horraires
* 
* 3 horaires (week_end / semaine / holiday)
* uint16_t
*/



#endif
