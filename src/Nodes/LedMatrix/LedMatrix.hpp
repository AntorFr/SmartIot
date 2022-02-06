#pragma once

#include <map>
#include "SmartIotLedMatrix.hpp"
#include "LedMatrixPattern.hpp"
#include "../Led/Led.hpp"

class SmartIotLedMatrix;

namespace SmartIotInternals {
    class LedPattern;
    class LedMatrixPattern;
}

class LedMatrix: public LedObject {
    public:
        LedMatrix(const uint16_t firstPos,const char* name,cLEDMatrixBase* const matrix);
        cLEDMatrixBase* getMatrix(){return _matrix;};
    protected:
        cLEDMatrixBase* const _matrix;

};




