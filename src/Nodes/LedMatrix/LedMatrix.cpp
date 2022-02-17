#include "LedMatrix.hpp"

using namespace SmartIotInternals;

LedMatrix::LedMatrix(const uint16_t firstPos, const char* name,cLEDMatrixBase* const matrix)
:LedObject(firstPos,matrix->Size(),name)
,_matrix(matrix){
  delete[] _leds;
  _leds = (*_matrix)[0];
  
  _patterns["clock"]= [](LedObject* ledMatrix) -> LedPattern* { return new ClockPattern(static_cast<LedMatrix*>(ledMatrix));};
  _patterns["rainbowClock"]= [](LedObject* ledMatrix) -> LedPattern* { return new RainbowClockPattern(static_cast<LedMatrix*>(ledMatrix)); };

  _autoplayList.insert(_autoplayList.end(), { "clock","rainbowClock" });

}
