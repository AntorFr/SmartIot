#include "LedMatrixPattern.hpp"

using namespace SmartIotInternals;

LedMatrixPattern::LedMatrixPattern(LedMatrix* const  obj)
    :LedPattern((LedObject* const)obj)
    ,_matrix(obj->getMatrix())
    {
    
}

void RainbowClockPattern::init(){
    _cTxt.SetFont(MatriseFontData);
    _cTxt.Init(_matrix, _matrix->Width(), _cTxt.FontHeight() + 1, 0, 0);

    fill_solid(_leds,_nbLed, CRGB::Black); 
    minutes = 100;
    show();
}

void RainbowClockPattern::display(){
    uint8_t min = Interface::get().getTime().getTm()->tm_min;

    uint8_t hue = beat8(_obj->getSpeed()/3);
    uint8_t delta = 2*_matrix->Width();

    if (minutes!=min || minutes==100){
        strcpy((char *)(_txt), Interface::get().getTime().getShortTime());
        minutes=min;

        
        
    }
    _cTxt.SetText(_txt, sizeof(_txt) - 1);
    _cTxt.SetTextColrOptions(COLR_HSV | COLR_GRAD_AH,hue,0xff,0xff,0xff,0xff,(hue+delta)%255);
    _cTxt.UpdateText();

    show();
    displayEffect();
}

void ClockPattern::init(){
    _cTxt.SetFont(MatriseFontData);
    _cTxt.Init(_matrix, _matrix->Width(), _cTxt.FontHeight() + 1, 0, 0);

    fill_solid(_leds,_nbLed, CRGB::Black); 
    minutes = 100;
    show();
}

void ClockPattern::display(){
    uint8_t min = Interface::get().getTime().getTm()->tm_min;

    if (minutes!=min || minutes==100){
        strcpy((char *)(_txt), Interface::get().getTime().getShortTime());
        minutes=min;

        _cTxt.SetText(_txt, sizeof(_txt) - 1);
        _cTxt.SetTextColrOptions(COLR_RGB,_obj->getColor().r,_obj->getColor().g,_obj->getColor().b);
        _cTxt.UpdateText();
        show();
 
    }    
    displayEffect();
}
