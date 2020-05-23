#include "LoopFunction.hpp"

using namespace SmartIotInternals;

LoopFunction::~LoopFunction() {
  stop();
}

void LoopFunction::start() {
  #ifdef ESP32
  if (_multitask){
      xTaskCreatePinnedToCore(_coreTaskImpl, "Task", 10000, this, 1,&_TaskHandle, 0); 
  } else if(_freq != 0) {
      _ticker.attach(_freq, +[](LoopFunction* loop) { loop->_function(); }, this);
  }
  #else
  if(_freq != 0) {
      _ticker.attach(_freq, _function);
  }
  #endif
  _init = true;
}

void LoopFunction::stop() {
  if (_init) {
    #ifdef ESP32
    if (_multitask){ vTaskDelete(_TaskHandle);}
    else if(_freq != 0) {_ticker.detach();}
    #else
    if (_freq != 0 ) {_ticker.detach();}
    #endif
    _init = false;
  }
}

void LoopFunction::setFunction(const OperationFunction& operationFunction,float freq ,bool multitask) {
  stop();
  _function = operationFunction;
  _freq = freq;
  _multitask = multitask;
  if (_freq != 0 || _multitask) {start();}
}

void LoopFunction::setFreq(float freq) {
  if (_freq != freq) {
    _freq = freq;
    reset();
  }
}

void LoopFunction::reset() {
  stop();
  if (_freq != 0) {start();}
}

void LoopFunction::run() {
  if (!_init){
    start();
  }

  if(_freq == 0 && !_multitask ){ _function();}
  else {return;}  //no need to run, function is launch by ticker or multitask.
}

#ifdef ESP32
void LoopFunction::_coreTask() {
  for(;;){ /* loop forever */
    _function();
    if (_freq != 0) { vTaskDelay(_freq * 1000 / portTICK_PERIOD_MS);} // similar to delay(_freq * 1000) if compiled with Arduino
  }
  vTaskDelete( NULL );
}

void LoopFunction::_coreTaskImpl(void* _this){
    static_cast<LoopFunction *>(_this)->_coreTask();
}
#endif

