
#define subjectBTtoMQTT  Base_Topic "sensors/"
#define TimeBtw_Read 2000 //55555 //define the time between 2 scans
#define Scan_duration 20 //define the time for a scan

#define delimiter "4f4b2b444953413a"
#define delimiter_length 16

#define BLE "BLE"
#define classicBT "BT"


/*-------------------HOME ASSISTANT ROOM PRESENCE ----------------------*/
// if not commented Home presence integration with HOME ASSISTANT is activated
#define subjectHomePresence Base_Topic "home_presence/" Gateway_Room // will send Home Assistant room presence message to this topic (first part is same for all rooms, second is room name)


struct decompose
{
  char subject[4];
  int start;
  int len;
  boolean reverse;
  char extract[60];
};
     
/*-------------------PIN DEFINITIONS----------------------*/
