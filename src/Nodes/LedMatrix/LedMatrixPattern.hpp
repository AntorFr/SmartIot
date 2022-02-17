#pragma once




#include "LedMatrix.hpp"
#include "../Led/Led.hpp"
#include "../Led/GradientPalettes.hpp"

class LedMatrix;
class LedObject;

namespace SmartIotInternals {
    class LedPattern;
    class LedMatrixPattern: public LedPattern {
        friend LedMatrix;
        public:
            LedMatrixPattern(LedMatrix* const  obj);
        protected:
            cLEDMatrixBase* const _matrix;
        };

    class ClockPattern : public LedMatrixPattern  {
        friend LedMatrix;
        public:
        ClockPattern(LedMatrix* const obj):LedMatrixPattern(obj){};
        protected:
            void init() override;
            void display() override;
            cLEDText _cTxt;
            unsigned char _txt[5+1];
            uint8_t minutes; 
    };
    class RainbowClockPattern : public LedMatrixPattern  {
        friend LedMatrix;
        public:
        RainbowClockPattern(LedMatrix* const obj):LedMatrixPattern(obj){};
        protected:
            void init() override;
            void display() override;
            cLEDText _cTxt;
            unsigned char _txt[5+1];
            uint8_t minutes; 
    };

} // namespace SmartIotInternals