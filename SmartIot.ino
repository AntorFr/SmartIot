#include "User_config.h"

#ifdef BT
  #include "config_BT.h"
#endif
#ifdef Watering
  #include "config_Watering.h"
#endif 
#ifdef LED
  #include "config_LED.h"
#endif 
#ifdef Roombot_wifi
  #include "Roombot_wifi.h"
#endif
#ifdef AmbientLight
  #include "config_Ambient_Light.h"
#endif 

#include <PubSubClient.h>
#include <ArduinoJson.h>

/*------------------------------------------------------------------------*/

// array to store previous received RFs, IRs codes and their timestamps
#define MQTT_MAX_PACKET_SIZE 1024 //useless change directly into pubsub.h https://github.com/knolleary/pubsubclient/issues/110
#define array_size 12 
unsigned long ReceivedSignal[array_size][2] ={{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};

//adding this to bypass the problem of the arduino builder issue 50
void callback(char*topic, byte* payload,unsigned int length);

boolean connectedOnce = false; //indicate if we have been connected once to MQTT
int failure_number = 0; // number of failure connecting to MQTT

#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #include "esp_system.h"
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  extern "C" {
    #include "user_interface.h"
  } 
#endif

#include <ArduinoOTA.h>

#ifdef MDNS_SD
  //#include <ESPmDNS.h>
#endif

WiFiClient eClient;

// client link to pubsub mqtt
PubSubClient client(eClient);

//MQTT last attemps reconnection date
unsigned long lastReconnectAttempt = 0;

//timers for LED indicators
unsigned long timer_led_receive = 0;
unsigned long timer_led_send = 0;
unsigned long timer_led_error = 0;

//Time used to wait for an interval before checking system measures
unsigned long timer_sys_measures = 0;

#ifdef ESP32 // Add watchdog on wifi reconnect
  const int wdtTimeout = 60000;  //time in ms to trigger the watchdog
  hw_timer_t *timer = NULL;
  
  void IRAM_ATTR resetModule() {
    ets_printf("reboot\n");
    esp_restart();
  }
#elif defined(ESP8266)
  void resetModule() {
    ets_printf("reboot\n");
    ESP.restart();
  }
#endif

boolean reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
        trc(F("MQTT connection...")); //F function enable to decrease sram usage
        if (client.connect(Gateway_Name, mqtt_user, mqtt_pass, will_Topic, will_QoS, will_Retain, will_Message)) {
        trc(F("Connected to broker"));
        failure_number = 0;
        // Once connected, publish an announcement...
        pub(will_Topic,Gateway_AnnouncementMsg,will_Retain);

        // Publish Sys measure with version
        stateMeasures(true);
        
        if (client.subscribe(subjectMQTTtoX)) {
          #ifdef Watering
            //client.subscribe(subjectWateringtoMQTT); // subject on which other SmartIot will publish, this SmartIot will store these msg and by the way don't republish them if they have been already published
          #endif
          trc(F("Subscription OK to the subjects"));
        }      
      } else {
        failure_number ++; // we count the failure
        trc(F("failure_number"));
        trc(failure_number);
        trc(F("failed, rc="));
        trc(client.state());
        trc(F("try again in 5s"));
        // Wait 5 seconds before retrying
        delay(5000);
  
        if (failure_number > maxMQTTretry){
          trc(F("failed connecting to mqtt"));
          setup_wifi();
        }
      }
  }
  return client.connected();
}

// Callback function, when the gateway receive an MQTT value on the topics subscribed this function is called
void callback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.
  trc(F("Hey I got a callback "));
  // Allocate the correct amount of memory for the payload copy
  byte* p = (byte*)malloc(length + 1);
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  // Conversion to a printable string
  p[length] = '\0';
  //launch the function to treat received data
  receivingMQTT(topic,(char *) p);
  // Free the memory
  free(p);
}

void setup() {
  //Launch serial for debugging purposes
  Serial.begin(SERIAL_BAUD);

  #ifdef ESP8266
  //Serial.end();
  //Serial.begin(SERIAL_BAUD, SERIAL_8N1, SERIAL_TX_ONLY);// enable on ESP8266 to free some pin
  #endif

  #ifdef ESP32
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, false); //set time in us
  timerAlarmEnable(timer);
  #endif
  
  setup_wifi();

  trc(F("SmartIoT mac: "));
  trc(WiFi.macAddress()); 

  trc(F("SmartIoT ip: "));
  trc(WiFi.localIP().toString());
  
  // Port defaults to 8266
  ArduinoOTA.setPort(ota_port);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(ota_hostname);

  // No authentication by default
  ArduinoOTA.setPassword(ota_password);

  ArduinoOTA.onStart([]() {
    trc(F("Start"));
  });
  ArduinoOTA.onEnd([]() {
    trc(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) trc(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) trc(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) trc(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) trc(F("Receive Failed"));
    else if (error == OTA_END_ERROR) trc(F("End Failed"));
  });
  ArduinoOTA.begin();

  long port;
  port = strtol(mqtt_port,NULL,10);
  trc(port);
 
  #ifdef mqtt_server_name // if name is defined we define the mqtt server by its name
   trc(F("Connecting to MQTT with mqtt hostname"));
   IPAddress mqtt_server_ip;
   WiFi.hostByName(mqtt_server_name, mqtt_server_ip);
   client.setServer(mqtt_server_ip, port);
   trc(mqtt_server_ip.toString());
  #else // if not by its IP adress
   trc(F("Connecting to MQTT by IP adress"));
   client.setServer(mqtt_server, port);
   trc(mqtt_server);
  #endif

  client.setCallback(callback);
  delay(1500);
  lastReconnectAttempt = 0;

  #ifdef BT
    setupBT();
  #endif
  #ifdef Watering
    setupWatering();
  #endif
  #ifdef Roombot_wifi
    setupRoombot_wifi();
  #endif
  #ifdef LED
    setupLED();
  #endif
  #ifdef AmbientLight
    setupAmbientLight();
  #endif

    
  trc(F("MQTT_MAX_PACKET_SIZE"));
  trc(MQTT_MAX_PACKET_SIZE);
  trc(F("Setup SmartIoT end"));
}

void setup_wifi() { 
  delay(10);
  WiFi.disconnect();
  int failureAttempt = 0; //DIRTY FIX ESP32 waiting for https://github.com/espressif/arduino-esp32/issues/653
  WiFi.mode(WIFI_STA);

  #ifdef ESP32
  WiFi.setHostname(Gateway_Name);
  #elif defined(ESP8266)  
  wifi_station_set_hostname(Gateway_Name);
  #endif
  
  // We start by connecting to a WiFi network
  trc(F("Connecting to "));
  trc(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  //WiFi.config(ip_adress,gateway_adress,subnet_adress,dns_adress); //Uncomment this line if you want to use advanced network config
    
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    trc(F("."));
    failureAttempt++; //DIRTY FIX ESP32
    if (failureAttempt > 30) setup_wifi(); //DIRTY FIX ESP32
  }
  trc(F("WiFi ok with manual config credentials"));
}

void loop() {
  unsigned long now = millis();
  #ifdef ESP32
  timerWrite(timer, 0);
  #endif

  #ifdef LED
    #ifdef LED_AUDIO
    read_audio();
    #endif
    display_led();
  #endif
  
  //MQTT client connexion management
  if (!client.connected()) { // not connected

    //RED ON
    digitalWrite(led_error, LOW);

    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else { //connected
    // MQTT loop
    //RED OFF
    if (now - timer_led_receive > 300) {
      timer_led_receive = now;
      digitalWrite(led_receive, HIGH);
    }
    digitalWrite(led_error, HIGH);
    connectedOnce = true;
    client.loop();


    ArduinoOTA.handle();
   
    #ifdef BT
      #ifndef multiCore
        if(BTtoMQTT())  
        trc(F("BTtoMQTT OK"));
      #endif
    #endif
    #ifdef Watering
      WateringtoMQTT();
    #endif
    #ifdef AmbientLight
      MeasureLightIntensity();
    #endif
    stateMeasures(false);
  }
}

void heartbeat(bool pub_verbose) {
  StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
  JsonObject SYSdata = jsonBuffer.to<JsonObject>();
  
  if(pub_verbose) SYSdata["version"] = SIot_VERSION;
  
  trc("Uptime (s)");    
  unsigned long uptime = millis()/1000;
  trc(uptime);
  SYSdata["uptime"] = uptime;
  trc("Remaining memory");
  uint32_t freeMem;
  freeMem = ESP.getFreeHeap();
  SYSdata["freeMem"] = freeMem;
  trc(freeMem);
  trc("RSSI");
  long rssi = WiFi.RSSI();
  SYSdata["rssi"] = rssi;
  trc(rssi);
  trc("SSID");
  String SSID = WiFi.SSID();
  SYSdata["SSID"] = SSID;
  trc(SSID);
  trc("Activated modules");
  String modules = "";

  #ifdef BT
   SYSdata["bt_id"] = getBTAddress();
  #endif
  
  #ifdef LED
    modules = modules + LED;
  #endif
  #ifdef BT
    modules = modules + BT;
  #endif
  #ifdef Watering
    modules = modules + Watering;
  #endif
  #ifdef Roombot_wifi
    modules = modules + Roombot_wifi;
  #endif      
  #ifdef AmbientLight
    modules = modules + AmbientLight;
  #endif 
  
  SYSdata["modules"] = modules;
  trc(modules);
  pub(subjectSYStoMQTT,SYSdata);
}

void stateMeasures(bool pub_verbose){
    unsigned long now = millis();
    if (now > (timer_sys_measures + TimeBetweenReadingSYS)) {//retriving value of memory ram every TimeBetweenReadingSYS
      timer_sys_measures = millis();
      heartbeat(pub_verbose);
    }
}

void receivingMQTT(char * topicOri, char * datacallback) {
  if (strstr(topicOri, subjectMultiGTWKey) != NULL) // storing received value so as to avoid publishing this value if it has been already sent by this or another OpenMQTTGateway
  {
    trc(F("Storing signal"));
    unsigned long data = 0;
    #ifdef jsonPublishing
      trc(F("Creating Json buffer"));
      StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, datacallback);
      if (error) {}
      else {
        JsonObject jsondata = jsonBuffer.as<JsonObject>();
        data =  jsondata["value"];
      }
    #endif

    #ifdef simplePublishing
      data = strtoul(datacallback, NULL, 10); // we will not be able to pass values > 4294967295
    #endif
    
    if (data != 0) {
      storeValue(data);
      trc(F("Data from JSON stored"));
    }
  }
  //YELLOW ON
  digitalWrite(led_send, LOW);

  trc(F("Creating Json buffer"));
  StaticJsonDocument<JSON_MSG_BUFFER> jsonBuffer;
  DeserializationError error = deserializeJson(jsonBuffer, datacallback);
  if (error) { // not a json object --> simple decoding
    #ifdef simplePublishing
      #ifdef Watering
       MQTTtoWatering(topicOri, datacallback);
      #endif
      #ifdef Roombot_wifi
       MQTTtoRoombot_wifi(topicOri, datacallback);
      #endif  
    #endif 
  } else { // json object ok -> json decoding
     JsonObject jsondata = jsonBuffer.as<JsonObject>();
     #ifdef jsonPublishing
      #ifdef Watering
       MQTTtoWatering(topicOri, jsondata);
      #endif
      #ifdef Roombot_wifi
       MQTTtoRoombot_wifi(topicOri, jsondata);
      #endif
      #ifdef LED
       MQTTtoLED(topicOri, jsondata);
      #endif
     #endif 
  }
  MQTTtoSYS(topicOri);
  //YELLOW OFF
  digitalWrite(led_send, HIGH);
}


void MQTTtoSYS(char * topicOri){
  String topic = topicOri;
  if (topic == MQTTtosubjectSYS){
    // we acknowledge the sending by publishing the value to an acknowledgement topic
    //trc(F("Request for heartbeat"));
    heartbeat(true);
  } else if (topic == MQTTtosubjectReboot || topic == MQTTtosubjectRebootAll) {
    //trc(F("Request for reboot"));
    resetModule();
  }
}
