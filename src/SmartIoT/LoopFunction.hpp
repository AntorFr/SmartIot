#pragma once

#include <Ticker.h>
#include "SmartIot/Datatypes/Callbacks.hpp"



namespace SmartIotInternals {

class LoopFunction {
 public:
  LoopFunction(const OperationFunction& operationFunction = [](){},float freq =0 ,bool multitask = false) { _function = operationFunction; _freq = freq; _multitask = multitask, _init=false;};
  ~LoopFunction();
  void run();
  void start();
  void stop();
  void setFreq(float freq=0);
  float getFreq() {return _freq;};
  bool getmultitask() {return _multitask;};
  void setFunction(const OperationFunction& operationFunction = [](){},float freq =0,bool multitask = false);
  void reset();
  
 private:
  Ticker _ticker;
  bool _init;
  float _freq;
  bool _multitask;
  OperationFunction _function;
  #ifdef ESP32
  void _coreTask();
  static void _coreTaskImpl(void*);
  xTaskHandle _TaskHandle;
  #endif
};

}  // namespace SmartIotInternals
