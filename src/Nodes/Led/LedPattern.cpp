#include "LedPattern.hpp"

using namespace SmartIotInternals;

LedPattern::LedPattern(const LedObject*  obj)
    :_show(false)
{
    _nbLed = obj->_nbLed;
    _leds = obj->_leds;
    _obj = obj;
}

void LedPattern::addGlitter(fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    _leds[ random16(_nbLed) ] += CRGB::White;
  }
}

void OffPattern::init(){
    fill_solid(_leds,_nbLed, CRGB::Black); 
    show();
}

void ColorPattern::init(){
    fill_solid(_leds,_nbLed, _obj->getColor()); 
    show();
}

void BlinkPattern::init(){
    _ticker.reset();
    _ticker.setPeriod(100000/_obj->getSpeed());
    show();
}

void BlinkPattern::display()
{
    if(_ticker)
    {
        //Interface::get().getLogger() << F("Blink from ") << _obj->getName()  << endl;
        if (_blink) { fill_solid(_leds,_nbLed, _obj->getColor());}
        else { fill_solid(_leds,_nbLed, CRGB::Black);}
        _blink= !_blink;
        show();
    }
}

void WipePattern::display()
{
    int p;
    fill_solid(_leds, _nbLed,CRGB::Black);
    int pos = beatsin16(_obj->getSpeed(),0,_nbLed*2-1);
     if(pos <= _nbLed) {
         p= pos;
         fill_solid(_leds, p,_obj->getColor());
     } else {
         p = pos - _nbLed;
         fill_solid(_leds+p,_nbLed-p,_obj->getColor());
     }
    show();
}

void LaserPattern::display()
{
    int p;
    fill_solid(_leds, _nbLed,CRGB::Black);
    int pos = beatsin16(_obj->getSpeed(),0,_nbLed*2+1);
     if(pos <= _nbLed) {
         p= pos;
         fill_solid(_leds, p,_obj->getColor());
     } else {
         p = pos -_nbLed-1;
         fill_solid(_leds+p, _nbLed-p,_obj->getColor());
     }
    show();
}

void BreathePattern::display()
{
    int hue = beatsin16(_obj->getSpeed(),0,255);
    fill_solid(_leds, _nbLed,_obj->getColor());
    for (uint16_t i = 0; i < _nbLed; i++) {
        _leds[i].nscale8(hue);
    }
    show();
}

void RainbowPattern::display()
{
    fill_rainbow(_leds, _nbLed,beat8(_obj->getSpeed()/3),255/(4*_nbLed));
    show();
}

void K2000Pattern::display()
{
    fadeToBlackBy(_leds, _nbLed,_obj->getSpeed()/2);
    int pos = beatsin16(_obj->getSpeed(),0,_nbLed-1);
    _leds[pos] = _obj->getColor();
    show();
}

void ComputerPattern::display()
{
    fadeToBlackBy(_leds, _nbLed,_obj->getSpeed()/2);
    int pos = beatsin16(_obj->getSpeed()/2,1,_nbLed);
    if(pos<=_nbLed/2){
    _leds[pos-1] = _obj->getColor();
    _leds[_nbLed-pos] = _obj->getColor();
    } else {

    }
    show();
}

void ConfettiPattern::display()
{
    fadeToBlackBy( _leds, _nbLed, 10);
    int pos = random16(_nbLed);
    _leds[pos] += CHSV( _obj->getHue() + random8(64), 200, 255);
    show();
}

void StarPattern::init(){
    _stars = new uint8_t[_nbLed];
    std::fill(_stars, _stars+_nbLed, 0);

    fill_solid(_leds,_nbLed, CRGB::Black); 
    show();
}
void StarPattern::display()
{

    for(uint16_t i=0; i<_nbLed; i++){
        if (_stars[i]==0 && random16(_nbLed*_obj->getSpeed())==0){
            _stars[i]=255;
        } 
        if (_stars[i] > 0) {
            float intensit = -1*pow(_stars[i]/180.1,2);
            _leds[i].r = _obj->getColor().r * exp(intensit);
            _leds[i].g = _obj->getColor().g * exp(intensit);
            _leds[i].b = _obj->getColor().b * exp(intensit);
        } else {
            _leds[i] = CRGB::Black;
        }
        if (_stars[i]%2 == 1) {_stars[i] = (_stars[i]>2)?_stars[i]-2:2;}
        else if (_stars[i] > 0) {_stars[i] = (_stars[i]<=254)?_stars[i]+2:0;}
    }
    show();
}

void PridePattern::display(){
    uint8_t sat8 = beatsin88( 87, 220, 250);
    uint8_t brightdepth = beatsin88( 341, 96, 224);
    uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
    uint8_t msmultiplier = beatsin88(147, 23, 60);

    uint16_t hue16 = sHue16;//gHue * 256;
    uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
    uint16_t ms = millis();
    uint16_t deltams = ms - sLastMillis ;

    sLastMillis  = ms;
    sPseudotime += deltams * msmultiplier;
    sHue16 += deltams * beatsin88( 400, 5,9);
    uint16_t brightnesstheta16 = sPseudotime;
  
    for( uint16_t i = 0 ; i < _nbLed; i++) {
        hue16 += hueinc16;
        uint8_t hue8 = hue16 / 256;

        brightnesstheta16  += brightnessthetainc16;
        uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

        uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
        uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
        bri8 += (255 - brightdepth);
        
        CRGB newcolor = CHSV( hue8, sat8, bri8);
        
        uint16_t pixelnumber = i;
        pixelnumber = (_nbLed-1) - pixelnumber;
        
        nblend( _leds[pixelnumber], newcolor, 64);
    }
    show();
}

void TwinkleFoxPattern::display()
{

  uint16_t PRNG16 = 11337; 
  uint32_t clock32 = millis();

  CRGB bg;
  if( (_autoBGColor) &&
      (gCurrentPalette[0] == gCurrentPalette[1] )) {
    bg = gCurrentPalette[0];
    uint8_t bglight = bg.getAverageLight();
    if( bglight > 64) {
      bg.nscale8_video( 16); // very bright, so scale to 1/16th
    } else if( bglight > 16) {
      bg.nscale8_video( 64); // not that bright, so scale to 1/4th
    } else {
      bg.nscale8_video( 86); // dim, scale to 1/3rd.
    }
  } else {
    bg = gBackgroundColor;
  }

  uint8_t backgroundBrightness = bg.getAverageLight();
  
  for( uint16_t i = 0 ; i < _nbLed; i++) {
    CRGB& pixel = _leds[i];
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16= PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF)>>4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    CRGB c = TwinkleFoxPattern::computeOneTwinkle( myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if( deltabright >= 32 || (!bg)) {
      pixel = c;
    } else if( deltabright > 0 ) {
      pixel = blend( bg, c, deltabright * 8);
    } else { 
      pixel = bg;
    }
  }
}

CRGB TwinkleFoxPattern::computeOneTwinkle( uint32_t ms, uint8_t salt)
{
  uint16_t ticks = ms >> (8-_speed);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8( slowcycle16);
  slowcycle16 =  (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);
  
  uint8_t bright = 0;
  if( ((slowcycle8 & 0x0E)/2) < _density) {
    bright = TwinkleFoxPattern::attackDecayWave8( fastcycle8);
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if( bright > 0) {
    c = ColorFromPalette( gCurrentPalette, hue, bright, NOBLEND);
    if(_coolLikeIncandescent) {
      TwinkleFoxPattern::coolLikeIncandescent( c, fastcycle8);
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}

void TwinkleFoxPattern::coolLikeIncandescent( CRGB& c, uint8_t phase)
{
  if( phase < 128) return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8( c.g, cooling);
  c.b = qsub8( c.b, cooling * 2);
}

uint8_t TwinkleFoxPattern::attackDecayWave8( uint8_t i)
{
  if( i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i/2));
  }
}


void RainbowSoundPattern::display()
{
    fill_rainbow(_leds, _nbLed,beat8(_obj->getSpeed()/3),255/(4*_nbLed));

    uint16_t audio = _obj->getAudio();
    uint16_t nbColorLed = _nbLed*_obj->getAudio()/(4*1024);
    uint8_t lastColorPct = map((_nbLed*_obj->getAudio()%(4*1024)), 0, (4*1024)-1, 0, 255);

    for(uint16_t i=0; i<_nbLed/2; i++){
        if(i>nbColorLed/2){
            _leds[(_nbLed/2+i)] = CRGB::Black;
            _leds[(_nbLed/2-i)-1] = CRGB::Black;
        } else if(i==nbColorLed/2) {
            _leds[(_nbLed/2+i)].nscale8(lastColorPct);
            _leds[(_nbLed/2-i)-1].nscale8(lastColorPct);
        }
    }
    show();
}



