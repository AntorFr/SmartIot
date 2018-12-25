#define STRIPPIN D1
#define AUDIO_AIN A0
#define AUDIO_DIN D4

#define LED_COUNT 100 // 100 Buffet // 60 Meuble TV // 60 wemos07 (LED Bar) // 100 Sapin
#define LED_DENSITY 100 
#define LED_AUDIO "AUDIO" // comment for sensor without audio
#define SYMETRICAL 1

#define subjecLEDtoMQTT  Base_Topic Gateway_Room "/LED" 
#define subjectMQTTtoLED  commands_Topic Gateway_Room "/LED"
#define subjectMQTTtoAllLED  commands_Topic "LED"

//*-------------DEFINE FILTER ADJUSTMENT ----------------*/

#define LED_filterVal 0.92
