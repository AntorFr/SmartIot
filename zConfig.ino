
void load_config() {
    load_setup_config();
    if (load_configfile()) {
        config_global._init=true;
        load_global_config();
    }

}

bool load_configfile(){
    File file = SPIFFS.open("config.json", "r");
    if (!file){
        trc(F("Config file is missing"));
        return get_config();
    } else {
        size_t size = file.size();
        if ( size == 0 ) {
            trc(F("Config file is empty"));
            return get_config();
    } else {
        DeserializationError err = deserializeJson(docConfig, file);
        if (err) {
            trc(F("deserializeJson() failed: "));
            trc(err.c_str());
        }
        file.close();
        return true;
    }
  }
}

void load_setup_config(){
    //default config before loading custom setting
    config_global.name = "SmartIoT_"+String(ESP.getChipId()));
    config_global.lastwill_topic = tolower("setup/status/up/"+config_global.name);

    config_global.config_topic = tolower("setup/config/"+String(ESP.getChipId()));
    config_global.ota_topic = tolower("setup/ota/"+String(ESP.getChipId()));


}

void load_global_config(){
    config_global.domaine = docConfig["global"]["domaine"].as<String>();
    config_global.room = docConfig["global"]["room"].as<String>();
    config_global.sensor = docConfig["global"]["sensor"].as<String>();
    config_global.name = docConfig["global"]["name"].as<String>();

    config_global.command_topic = tolower(config_global.domaine+"/commands/");
    config_global.lastwill_topic = tolower(config_global.domaine+"/status/up/"+config_global.room+"/"+config_global.sensor);
    config_global.heartbeat_topic = tolower(config_global.command_topic + "heartbeat");
    config_global.poisonpill_command_all_topic = tolower(config_global.command_topic + "poisonpill");
    config_global.poisonpill_command_topic = tolower(config_global.poisonpill_command_all_topic + "/"+config_global.room+"/"+config_global.sensor);
}

bool get_config(){
    trc(F("Ask for config setings"));
    //TODO
    return false;
}

void readMQTT_config(JsonObject& configdata){
    String jsondoc="";
    trc(F("config received "));
    File  f = SPIFFS.open(fileconfig, "w");
    serializeJson(configdata,jsondoc);
    f.print(jsondoc);  // sauvegarde de la chaine
    f.close();
    trc(F("config loaded, restart module"));
    ESP.restart()
}


}