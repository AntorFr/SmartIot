#pragma once

#include <time.h>
#ifdef ESP8266
  #include <coredecls.h>  // settimeofday_cb()
  #include <TZ.h>
#endif
                
#include "Datatypes/Interface.hpp"

namespace SmartIotInternals {
class Time {
 public:
   Time();
   void init();
   bool isReady() {return time_ready; }
   const char* getTime();
   const char* getShortTime();
   const char* getDate();
   const char* getIso();
   const char* getIsoBootTime();
   time_t getBootTime() {if(time_ready) {return _boot;} else { return 0;}}
   void getStrfTime(char* ptr, size_t maxsize, const char* format); //https://www.cplusplus.com/reference/ctime/strftime/
   tm* getTm();
   
    
 private:
 bool time_ready;
 time_t _now;
 tm* _tm;
 time_t _boot;
 tm* _boot_tm; 
 void _time_is_set();
 void _refresh();
 void _sync();
 char _buf[30];

};
} //namespace SmartIotInternals