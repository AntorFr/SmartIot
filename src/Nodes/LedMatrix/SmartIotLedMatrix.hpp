#pragma once

#include <ArduinoJson.h>
#include <Ticker.h>
#define FASTLED_INTERNAL //avoid prama message
#define FASTLED_ALLOW_INTERRUPTS 0 // 0 avoid esp8266 flickering
#define INTERRUPT_THRESHOLD 1


#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>
#include <algorithm>

#include "../SmartIotLed.h"
#include <SmartIot.h>

//#include "GradientPalettes.hpp"
//#include "SmartIotNode.hpp"
#include "../../SmartIot/Datatypes/Interface.hpp"
#include "LedMatrixPattern.hpp"


using namespace SmartIotInternals;

namespace SmartIotInternals {
    class LedMatrixPattern;
}

class SmartIotLedMatrix : public SmartIotLed  {
    public:
    SmartIotLedMatrix(const char* id, const char* name, const char* type = "ledMatrix"):SmartIotLed(id,name,type){};
    LedObject* createObject(const uint16_t firstPos,const char* name,cLEDMatrixBase* matrix);
    
    template<int16_t tWidth, int16_t tHeight, MatrixType_t tMType> 
    void setupMatrix(const char* name){ 
        _matrix = new cLEDMatrix<tWidth, -tHeight, tMType>();
        createObject(0,name,_matrix);
    }

    cLEDMatrixBase* getMatrix() {return _matrix;}

    protected:

        
        
    private:
        cLEDMatrixBase* _matrix;

};

/*
enum EOrder {
	RGB=0012,
	RBG=0021,
	GRB=0102,
	GBR=0120,
	BRG=0201,
	BGR=0210
};
*/