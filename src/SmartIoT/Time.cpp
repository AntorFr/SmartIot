#include "Time.hpp"


using namespace SmartIotInternals;

Time::Time() : time_ready(false) {}

void Time::init(){
    configTime(TZ_Europe_Paris, "pool.ntp.org");
    settimeofday_cb([=]() {this->_time_is_set();});
}

void Time::_time_is_set(){
    time_ready=true;
    _sync();
    Interface::get().getLogger() << F("✔ NTP time synchronized. ") << endl;
    Interface::get().getLogger() << F(" Date: ") << ctime(&_now) << endl;
    Interface::get().event.type = SmartIotEventType::NTP_SYNCHRONIZED;
    Interface::get().eventHandler(Interface::get().event);
}

void Time::_sync(){
    time(&_now);
    _tm = localtime(&_now);
}

char* Time::getTime(){
    _sync();
    char buf [8+1];
    strftime(buf,9,"%T",_tm);
    return buf;
}

char* Time::getShortTime(){
    _sync();
    char buf [5+1];
    strftime(buf,6,"%R",_tm);
    return buf;
}

char* Time::getDate(){
    _sync();
    char buf [10+1];
    strftime(buf,11,"%d/%m/%Y",_tm);
    return buf;
}

char* Time::getIso(){
    _sync();
    char buf [19+1];
    strftime(buf,20,"%F %T",_tm); //2001-08-23 14:55:02
    return buf;

}

void Time::getStrfTime(char* ptr, size_t maxsize, const char* format){
    _sync();
    strftime(ptr,maxsize,format,_tm);
}


tm* Time::getTm(){
    _sync();
    return _tm;
}



