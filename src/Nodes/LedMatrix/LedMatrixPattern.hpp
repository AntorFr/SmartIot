#pragma once


//#include "../Led/GradientPalettes.hpp"
#include "LedMatrix.hpp"
class LedMatrix;

namespace SmartIotInternals {
    class LedPattern;
    class LedMatrixPattern: public LedPattern {
        friend LedMatrix;
        public:
            LedMatrixPattern(const LedMatrix*  obj):LedPattern(obj),_matrix(obj->_matrix){};
        protected:
            cLEDMatrixBase* _matrix;
        };

    class ClockPattern : public LedMatrixPattern  {
        friend LedObject;
        public:
        ClockPattern(const LedMatrix* obj):LedMatrixPattern(obj){};
        protected:
            void init() override;
            void display() override;
            cLEDText _cTxt;
            unsigned char _txt[30];
    };

} // namespace SmartIotInternals