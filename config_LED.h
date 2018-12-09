
#define STRIPPIN D1
#define AUDIO_AIN A0
#define AUDIO_DIN D4

#define LED_COUNT 100 // 100 wemos03 (buffet) // 60 wemos04 (meuble TV) // 120 wemos05 (TV)  // 144 wemos06 (bureau) // 60 wemos07 (LED Bar) //144 wemos08 (chambre Emilie)
#define LED_DENSITY 100
#define LED_AUDIO "AUDIO" // comment for sensor without audio
#define SYMETRICAL 1

#define subjecLEDtoMQTT  Base_Topic Gateway_Room "/LED" 
#define subjectMQTTtoLED  commands_Topic Gateway_Room "/LED"

//*-------------DEFINE FILTER ADJUSTMENT ----------------*/

#define LED_filterVal 0.92
#define LED_volume_filter 1
