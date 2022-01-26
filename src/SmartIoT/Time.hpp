#pragma once

#include <time.h>
#include <coredecls.h>                  // settimeofday_cb()
#include <TZ.h>

#include "Datatypes/Interface.hpp"

namespace SmartIotInternals {
class Time {
 public:
   Time();
   void init();
   bool status() {return time_ready; }
   char* getTime();
   char* getShortTime();
   char* getDate();
   char* getIso();
   void getStrfTime(char* ptr, size_t maxsize, const char* format); //https://www.cplusplus.com/reference/ctime/strftime/
   tm* getTm();
   
    
 private:
 bool time_ready;
 time_t _now;
 tm* _tm; 
 void _time_is_set();
 void _refresh();
 void _sync();

};
} //namespace SmartIotInternals