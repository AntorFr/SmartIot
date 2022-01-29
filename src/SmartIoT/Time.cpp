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
    Interface::get().getLogger() << F(" Date: ") << ctime(&_now);
    Interface::get().event.type = SmartIotEventType::NTP_SYNCHRONIZED;
    Interface::get().eventHandler(Interface::get().event);

    Interface::get().getUpTime().update();
    time_t upsec = Interface::get().getUpTime().getSeconds();
    _boot = _now - upsec;
    _boot_tm = localtime(&_boot);
}

void Time::_sync(){
    time(&_now);
    _tm = localtime(&_now);
}

char* Time::getTime(){
    _sync();
    strftime(_buf,30,"%T",_tm);
    return _buf;
}

char* Time::getShortTime(){
    _sync();
    strftime(_buf,30,"%R",_tm);
    return _buf;
}

char* Time::getDate(){
    _sync();
    strftime(_buf,30,"%d/%m/%Y",_tm);
    return _buf;
}

char* Time::getIso(){
    _sync();
    strftime(_buf,30,"%F %T",_tm); //2001-08-23 14:55:02
    return _buf;

}

char* Time::getIsoBootTime(){
    if(time_ready){
        strftime(_buf,30,"%F %T",_boot_tm); //2001-08-23 14:55:02
        return _buf; 
    } else {
        return getIso();
    }
}

void Time::getStrfTime(char* ptr, size_t maxsize, const char* format){
    _sync();
    strftime(ptr,maxsize,format,_tm);
}


tm* Time::getTm(){
    _sync();
    return _tm;
}



