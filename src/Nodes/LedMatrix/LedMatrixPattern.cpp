#include "LedMatrixPattern.hpp"

using namespace SmartIotInternals;


void ClockPattern::init(){
    _cTxt.SetFont(MatriseFontData);
    _cTxt.Init(_matrix, _matrix->Width(), _cTxt.FontHeight() + 1, 0, 0);
    strcpy_P((char *)(_txt), PSTR(EFFECT_HSV_AH));
    strcat_P((char *)(_txt), PSTR("\x00\xff\xff\xff\xff\xff"));
    strcat((char *)(_txt), Interface::get().getTime().getShortTime());
}

void ClockPattern::display(){
    _cTxt.SetText(_txt, sizeof(_txt) - 1);
    _cTxt.UpdateText();
    show();
    displayEffect();
}
