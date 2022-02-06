#include "LedMatrixPattern.hpp"

using namespace SmartIotInternals;

LedMatrixPattern::LedMatrixPattern(LedMatrix* const  obj)
    :LedPattern((LedObject* const)obj)
    ,_matrix(obj->getMatrix())
    {
    
}

void ClockPattern::init(){
    _cTxt.SetFont(MatriseFontData);
    _cTxt.Init(_matrix, _matrix->Width(), _cTxt.FontHeight() + 1, 0, 0);

    strcpy((char *)(_txt), Interface::get().getTime().getShortTime());
    minutes = Interface::get().getTime().getTm()->tm_min;

}

void ClockPattern::display(){

    uint8_t min = Interface::get().getTime().getTm()->tm_min;
    if (minutes!=min){
        strcpy((char *)(_txt), Interface::get().getTime().getShortTime());
        minutes=min;
    }

    uint8_t hue = beat8(_obj->getSpeed()/3);
    uint8_t delta = 2*_matrix->Width();

    _cTxt.SetText(_txt, sizeof(_txt) - 1);
    _cTxt.SetTextColrOptions(COLR_HSV | COLR_GRAD_AH,hue,0xff,0xff,0xff,0xff,(hue+delta)%255);
    _cTxt.UpdateText();

    show();
    displayEffect();
}
