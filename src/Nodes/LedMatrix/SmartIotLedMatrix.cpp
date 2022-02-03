#include "SmartIotLedMatrix.hpp"

using namespace SmartIotInternals;

LedObject* SmartIotLedMatrix::createObject(const uint16_t firstPos,const char* name,cLEDMatrixBase* matrix){
    LedObject* obj = new LedMatrix(firstPos,name,matrix);
    objects.push_back(obj);
    return obj;  
}




