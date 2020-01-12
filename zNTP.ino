#ifdef NTP
  #include <time.h>

  //TODO > Implement embeded lib for ESP32 https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Time/SimpleTime/SimpleTime.ino
  struct tm tm;
 
  void setupNTP()
  {
      configTime(0, 0, "pool.ntp.org");  
      settimeofday(0, nullptr);
      setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
      tzset(); 
  }
  
  void loopNTP()
  {
    time_t tnow = time(nullptr);
    localtime_r(&tnow, &tm);
    /*
    trc(String(ctime(&tnow)));
    
    trc(tm.tm_year);
    trc(tm.tm_mon);
    trc(tm.tm_mday);
    trc(tm.tm_hour);
    trc(tm.tm_min);
    trc(tm.tm_sec);
    trc(tm.tm_wday);
    */
    
  }

  



#endif
