
/*-------------------VERSION----------------------*/
#define SIot_VERSION "1.8"

#define wifi_ssid "<SSID>"
#define wifi_password "<PASSWORD>"

#define Gateway_Room "<ROOM_NAME>"
#define Gateway_Sensor "<SENSOR_NAME>"
#define Gateway_Name "SmartIoT_" Gateway_Room "_" Gateway_Sensor


//*-------------DEFINE THE MODULES ----------------*/
//#define BT     "BT"  
//#define Watering  "Watering"  
//#define Roombot_wifi "Roombot_wifi"
//#define LED "LED"
//#define AmbientLight "AmbientLight"
//#define Impulse "Impulse"
//#define ZsensorDS18XX "ZsensorDS18XX"
/*-------------------ACTIVATE TRACES----------------------*/
#define TRACE 1  // commented =  trace off, uncommented = trace on

/*-------------DEFINE YOUR MQTT PARAMETERS BELOW----------------*/
//MQTT Parameters definition
//#define mqtt_server_name "www.mqtt_broker.com" // instead of defining the server by its IP you can define it by its name, uncomment this line and set the correct MQTT server host name
char mqtt_user[20] = "<MQTT_USER>"; // not compulsory only if your broker needs authentication
char mqtt_pass[30] = "<MQTT_PASSWORD>"; // not compulsory only if your broker needs authentication
char mqtt_server[40] = "<MQTT_SERVER_IP>";
char mqtt_port[6] = "1883";


//uncomment the line below to integrate msg value into the subject when receiving
//#define valueAsASubject true

/*-------------DEFINE YOUR ADVANCED NETWORK PARAMETERS BELOW----------------*/
//#define MDNS_SD //uncomment if you  want to use mdns for discovering automatically your ip server, please note that MDNS with ESP32 can cause the BLE to not work
//#define cleanFS true //uncomment if you want to clean the ESP memory and reenter your credentials
#define maxMQTTretry 4 //maximum MQTT connection attempts before going to wifi setup

/*-------------DEFINE YOUR MQTT ADVANCED PARAMETERS BELOW----------------*/
#define Base_Topic "home/"
#define commands_Topic Base_Topic "commands/"

//#define version_Topic  Base_Topic Gateway_Name "/version"
#define will_Topic  Base_Topic "last_wills/" Gateway_Name
#define will_QoS 0
#define will_Retain true
#define will_Message "{\"status\":\"Offline\"}"
#define Gateway_AnnouncementMsg "{\"status\":\"Online\"}"

#define jsonPublishing true //comment if you don't want to use Json  publishing  (one topic for all the parameters)
//example home/OpenMQTTGateway_ESP32_DEVKIT/BTtoMQTT/4XXXXXXXXXX4 {"rssi":-63,"servicedata":"fe0000000000000000000000000000000000000000"}

//#define simplePublishing true //comment if you don't want to use simple publishing (one topic for one parameter)
//example 
// home/OpenMQTTGateway_ESP32_DEVKIT/BTtoMQTT/4XXXXXXXXXX4/rssi -63.0
// home/OpenMQTTGateway_ESP32_DEVKIT/BTtoMQTT/4XXXXXXXXXX4/servicedata fe0000000000000000000000000000000000000000

/*-------------DEFINE YOUR OTA PARAMETERS BELOW----------------*/
#define ota_hostname Gateway_Name
#define ota_password "SmartIoT"
#define ota_port 8266

/*-------------DEFINE PINs FOR STATUS LEDs----------------*/
#define led_receive LED_BUILTIN
#define led_send LED_BUILTIN
#define led_error LED_BUILTIN


#define SERIAL_BAUD 115200
/*--------------MQTT general topics-----------------*/
// global MQTT subject listened by the gateway to execute commands (send RF, IR or others)
#define subjectMQTTtoX  Base_Topic "commands/#"
#define subjectMultiGTWKey "toMQTT"

//variables to avoid duplicates
#define time_avoid_duplicate 3000 // if you want to avoid duplicate mqtt message received set this to > 0, the value is the time in milliseconds during which we don't publish duplicates

//uncomment to use multicore function of ESP32 for BLE
#ifdef ESP32
  #define multiCore //comment to don't use multicore function of ESP32 for BLE
#endif

#define JSON_MSG_BUFFER 512 // Json message max buffer size, don't put 1024 or higher it is causing unexpected behaviour on ESP8266

#define TimeBetweenReadingSYS 120000 // time between system readings (like memory)
#define subjectSYStoMQTT  Base_Topic "heartbeat/" Gateway_Name
#define MQTTtosubjectSYS commands_Topic "heartbeat"
#define MQTTtosubjectReboot commands_Topic "poisonpill/" Gateway_Name
#define MQTTtosubjectRebootAll commands_Topic "poisonpill"
