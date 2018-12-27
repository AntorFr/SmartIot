
//trace
void trc(String msg){
  #ifdef TRACE
  Serial.println(msg);
  #endif
}

//trace
void trc(char* msg){
  #ifdef TRACE
  Serial.println(msg);
  #endif
}

void trc(int msg){
  #ifdef TRACE
  Serial.println(msg);
  #endif
}

void trc(unsigned int msg){
  #ifdef TRACE
  Serial.println(msg);
  #endif
}

void trc(long msg){
  #ifdef TRACE
  Serial.println(msg);
  #endif
}

void trc(unsigned long msg){
  #ifdef TRACE
  Serial.println(msg);
  #endif
}

void trc(double msg){
  #ifdef TRACE
  Serial.println(msg);
  #endif
}

void trc(float msg){
  #ifdef TRACE
  Serial.println(msg);
  #endif
}



void storeValue(unsigned long MQTTvalue){
    unsigned long now = millis();
    // find oldest value of the buffer
    int o = getMin();
    trc(F("Min ind: "));
    trc(o);
    // replace it by the new one
    ReceivedSignal[o][0] = MQTTvalue;
    ReceivedSignal[o][1] = now;
    trc(F("store code :"));
    trc(String(ReceivedSignal[o][0])+"/"+String(ReceivedSignal[o][1]));
    trc(F("Col: val/timestamp"));
    for (int i = 0; i < array_size; i++)
    {
      trc(String(i) + ":" + String(ReceivedSignal[i][0])+"/"+String(ReceivedSignal[i][1]));
    }
}

int getMin(){
  unsigned int minimum = ReceivedSignal[0][1];
  int minindex=0;
  for (int i = 0; i < array_size; i++)
  {
    if (ReceivedSignal[i][1] < minimum) {
      minimum = ReceivedSignal[i][1];
      minindex = i;
    }
  }
  return minindex;
}

boolean isAduplicate(unsigned long value){
trc(F("isAduplicate?"));
// check if the value has been already sent during the last time_avoid_duplicate
for (int i = 0; i < array_size;i++){
 if (ReceivedSignal[i][0] == value){
      unsigned long now = millis();
      if (now - ReceivedSignal[i][1] < time_avoid_duplicate){ // change
      trc(F("--don't pub. duplicate--"));
      return true;
    }
  }
}
return false;
}

void extract_char(char * token_char, char * subset, int start ,int l, boolean reverse, boolean isNumber){
    char tmp_subset[l+1];
    memcpy( tmp_subset, &token_char[start], l );
    tmp_subset[l] = '\0';
    if (isNumber){
      char tmp_subset2[l+1];
      if (reverse) revert_hex_data(tmp_subset, tmp_subset2, l+1);
      else strncpy( tmp_subset2, tmp_subset , l+1);
      long long_value = strtoul(tmp_subset2, NULL, 16);
      sprintf(tmp_subset2, "%ld", long_value);
      strncpy( subset, tmp_subset2 , l+1);
    }else{
      if (reverse) revert_hex_data(tmp_subset, subset, l+1);
      else strncpy( subset, tmp_subset , l+1);
    }
    subset[l] = '\0';
}

void revert_hex_data(char * in, char * out, int l){
  //reverting array 2 by 2 to get the data in good order
  int i = l-2 , j = 0; 
  while ( i != -2 ) {
    if (i%2 == 0) out[j] = in[i+1];
    else  out[j] = in[i-1];
    j++;
    i--;
  }
  out[l-1] = '\0';
}

int strpos(char *haystack, char *needle) //from @miere https://stackoverflow.com/users/548685/miere
{
   char *p = strstr(haystack, needle);
   if (p)
      return p - haystack;
   return -1;
}

bool to_bool(String const& s) { // thanks Chris Jester-Young from stackoverflow
     return s != "0";
}


void pub(char * topic, char * payload, boolean retainFlag){
  bool res = client.publish(topic, payload, retainFlag);
  if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(char * topicori, JsonObject& data){
    String topic = topicori;
    #ifdef valueAsASubject
      unsigned long value = data["value"];
      if (value != 0){
        topic = topic + "/"+ String(value);
      }
    #endif
    
    #ifdef jsonPublishing
      char JSONmessageBuffer[JSON_MSG_BUFFER];
      trc(F("Pub json into:"));
      trc(topic);
      serializeJson(data, JSONmessageBuffer);
      trc(JSONmessageBuffer);
      pub(topic, JSONmessageBuffer);
    #endif

    #ifdef simplePublishing
      trc(F("Pub data per topic"));
      // Loop through all the key-value pairs in obj 
      for (auto p : data) {
        if (p.value().is<unsigned long>() || p.value().is<int>()) {
          trc(p.key().c_str());
          trc(p.value().as<unsigned long>());
          if (strcmp(p.key().c_str(), "value") == 0){ // if data is a value we don't integrate the name into the topic
            pub(topic,p.value().as<unsigned long>());
          }else{ // if data is not a value we integrate the name into the topic
            pub(topic + "/" + p.key().c_str(),p.value().as<unsigned long>());
          }
        } else if (p.value().is<float>()) {
          trc(p.key().c_str());
          trc(p.value().as<float>());
          pub(topic + "/" + p.key().c_str(),p.value().as<float>());
        } else if (p.value().is<char*>()) {
          trc(p.key().c_str());
          trc(p.value().as<const char*>());
          pub(topic + "/" + p.key().c_str(),p.value().as<const char*>());
        }
      }
    #endif
}

void pub(char * topic, char * payload){
    bool res = client.publish(topic, payload);
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(char * topic, String payload){
    bool res = client.publish(topic,(char *)payload.c_str());
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(String topic, String payload){
    bool res = client.publish((char *)topic.c_str(),(char *)payload.c_str());
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(String topic, char *  payload){
    bool res = client.publish((char *)topic.c_str(),payload);
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(String topic, int payload){
    char val[12];
    sprintf(val, "%d", payload);
    bool res = client.publish((char *)topic.c_str(),val);
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(String topic, float payload){
    char val[12];
    dtostrf(payload,3,1,val);
    bool res = client.publish((char *)topic.c_str(),val);
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(char * topic, float payload){
    char val[12];
    dtostrf(payload,3,1,val);
    bool res = client.publish(topic,val);
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(char * topic, int payload){
    char val[6];
    sprintf(val, "%d", payload);
    bool res = client.publish(topic,val);
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(char * topic, unsigned int payload){
    char val[6];
    sprintf(val, "%u", payload);
    bool res = client.publish(topic,val);
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(char * topic, unsigned long payload){
    char val[11];
    sprintf(val, "%lu", payload);
    bool res = client.publish(topic,val);
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

void pub(String topic, unsigned long payload){
    char val[11];
    sprintf(val, "%lu", payload);
    bool res = client.publish((char *)topic.c_str(),val);
    if (!res) {
      trc(F("> Error - MQTT Disconected"));
    }
}

#ifdef ESP32
void DesableBLE(){
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
}

void DesableBTC(){
   esp_bt_controller_disable();
   esp_bt_controller_deinit();
}

void DesableBT(){
   DesableBLE();
   DesableBTC();
}
#endif //ESP32

void DesableWifi(){
 #ifdef ESP32
  //esp_wifi_stop();
  WiFi.mode(WIFI_OFF);
 #elif defined(ESP8266) 
  WiFi.mode(WIFI_OFF);
 #endif
}

void DesableNetwork(){
  #ifdef ESP32
  DesableBT();
  #endif //ESP32
  DesableWifi();
}

void DeepSleep(int duration){
 #ifdef ESP32
  esp_sleep_enable_timer_wakeup(duration * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
 #elif defined(ESP8266) 
  ESP.deepSleep(duration * uS_TO_S_FACTOR);
 #endif
}
