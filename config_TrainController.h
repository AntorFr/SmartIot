#define subjectMQTTtoControlTrain  "SmallCity/commands/train/"
#define subjectMQTTtoRailSwitch  "smallcity/commands/railswitch"

typedef struct TrainConfig {
  const String name;
  const byte pin;
  const int8_t min;
  const int8_t max;
  const uint16_t midle;
  int8_t lastspeed;
  unsigned long last_send; 
};

typedef TrainConfig TrainConfigs[];

TrainConfigs trains = {
  {"TGV",A4,-55,55,2020,0,0},
  {"TER",A5,-55,55,1825,0,0}
};

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
const char keys[ROWS][COLS] = {
    {'a','b','c','d'},
    {'e','f','g','h'},
    {'i','j','k','l'},
    {'m','n','o','p'}
};

byte rowPins[ROWS] = {14, 27, 26, 25}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {19, 18, 5,17}; //connect to the column pinouts of the keypad
