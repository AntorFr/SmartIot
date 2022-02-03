#include "LedMatrix.hpp"

using namespace SmartIotInternals;

LedMatrix::LedMatrix(const uint16_t firstPos, const char* name, cLEDMatrixBase* matrix):LedObject(firstPos,matrix->Size(),name),_matrix(matrix){

  _patterns["clock"]= [](LedMatrix* ledObj) -> LedPattern* { return new ClockPattern(ledObj); };

  _autoplayList.insert(_autoplayList.end(), { "clock" });

}
