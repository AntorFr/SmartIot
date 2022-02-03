#pragma once

#include <map>
#include "SmartIotLedMatrix.hpp"
#include "../Led/Led.hpp"

class SmartIotLedMatrix;

namespace SmartIotInternals {
    class LedPattern;
    class LedMatrixPattern;
}

class LedMatrix: public LedObject {
    friend SmartIotLedMatrix;
    friend LedMatrixPattern;
    public:
        LedMatrix(const uint16_t firstPos,const char* name,cLEDMatrixBase* matrix);
    protected:
        cLEDMatrixBase* _matrix;

};




