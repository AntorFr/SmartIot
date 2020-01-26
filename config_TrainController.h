#define subjectMQTTtoControlTrain  "SmallCity/commands/train/"


typedef struct TrainConfig {
  const String name;
  const byte pin;
  const int8_t min;
  const int8_t max;
  const uint16_t midle;
  int8_t lastspeed; 
};

typedef TrainConfig TrainConfigs[];

TrainConfigs trains = {
  {"TGV",A4,-55,55,2450,0},
  {"TER",A5,-55,55,2450,0}
};
