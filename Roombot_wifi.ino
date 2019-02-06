#ifdef Roombot_wifi

#include <HTTPClient.h>

HTTPClient http;

void setupRoombot_wifi(){
}

#ifdef simplePublishing
void MQTTtoRoombot_wifi(char * topicOri, char * datacallback){
  MQTTtoRoombot_wifi(topicOri);
}
#endif

#ifdef jsonPublishing
void MQTTtoRoombot_wifi(char * topicOri, JsonObject& Wateringdata){
  MQTTtoRoombot_wifi(topicOri);
}
#endif

void MQTTtoRoombot_wifi(char * topicOri){
  String topic = topicOri;
  if (topic == subjectMQTTtoRoombot){
    // we acknowledge the sending by publishing the value to an acknowledgement topic
    trc(F("> Config Roombot Start"));
    pub(subjectRoombottoMQTT, F("{\"log\": \"Start config roombot\"}"));
    config_roombot();
  }
}

void config_roombot(){
  if(!Search_Roombot_SSID()) {
    trc(F("** Roombot SSID not found **"));
    setup_wifi();
    reconnect();
    pub(subjectRoombottoMQTT, F("{\"log\": \"Error - Roombot SSID not found\"}"));
    return;
  }
  
  if(!Connect_Roombot_wifi()) {
    trc(F("** Error for connect to Roombot wifi **"));
    setup_wifi();
    reconnect();
    pub(subjectRoombottoMQTT, F("{\"log\": \"Error - Roombot connecting Roombot wifi\"}"));
    return;
  }
  
  if (!Send_Config()) {
    trc(F("** Error for connect to Roombot wifi **"));
    setup_wifi();
    reconnect();
    pub(subjectRoombottoMQTT, F("{\"log\": \"Error - Roombot sending Config\"}"));
    return;
  } 
  
  trc(F("** Config sent **"));
  setup_wifi();
  reconnect();
  pub(subjectRoombottoMQTT, F("{\"log\": \"OK - Config sent\"}"));    
}

bool Search_Roombot_SSID() {
  WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  //delay(100);
  WiFi.scanDelete();

  trc(F("** Scan Networks **"));
  byte numSsid = WiFi.scanNetworks();
  bool wifi_found = false;

  for (int thisNet = 0; thisNet<numSsid; thisNet++) {
    if(WiFi.SSID(thisNet) == roombot_SSID){
      trc(F("** Roombot wifi found ! start config **"));
      return true;
    }
  }
  return false;
}

bool Connect_Roombot_wifi() {  
  delay(10);
  WiFi.disconnect();
  int failureAttempt = 0; //DIRTY FIX ESP32 waiting for https://github.com/espressif/arduino-esp32/issues/653
  WiFi.mode(WIFI_STA);

  WiFi.begin(roombot_SSID);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    trc(F("."));
    failureAttempt++; //DIRTY FIX ESP32
    if (failureAttempt > 30) return false; //DIRTY FIX ESP32
  }
  trc(F("WiFi connected Roombot"));
  return true;
}

bool Send_Config(){
  bool send_ok = false;

  String url = "http://";
  url += WiFi.gatewayIP().toString(); 
  url +="/configure.json";

  trc(url);
  
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(roombot_configString);
  trc(httpResponseCode);   //Print return code

  String response = http.getString(); 
  trc(response);  //Print request answer
  
  if(httpResponseCode>0){
    String response = http.getString();                       //Get the response to the request
    trc(httpResponseCode);   //Print return code
    trc(response);  //Print request answer
    send_ok = true;
   }else{
    trc(F("Error on sending POST: "));
    trc(httpResponseCode);
   }
   http.end();  //Free resources
   return send_ok;
}

#endif
