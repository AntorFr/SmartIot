#define STRIPPIN D1
#define AUDIO_AIN A0
#define AUDIO_DIN D4

#define LED_COUNT 200 // 100 Buffet // 60 Meuble TV // 60 wemos07 (LED Bar) // 200 Sapin // Lampe Emilie 36
#define LED_DENSITY 15
//#define LED_AUDIO "AUDIO" // comment for sensor without audio
#define Fast_LED "FL" // uncomment for Smart stars light
#define SYMETRICAL 1

#define subjecLEDtoMQTT  Base_Topic Gateway_Room "/LED" 
#define subjectMQTTtoLED  commands_Topic Gateway_Room "/LED"
#define subjectMQTTtoAllLED  commands_Topic "LED"

//*-------------DEFINE FILTER ADJUSTMENT ----------------*/

#define LED_filterVal 0.92

struct led_obj {
  uint8_t start_point;
  uint8_t nb_led;
};
/*

*/
