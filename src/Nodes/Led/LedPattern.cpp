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

void LedPattern::addPowerCut(fract16 chanceOfPowercut) 
{
  if( random16() < chanceOfPowercut) {
    fill_solid(_leds,_nbLed, CRGB::Black);
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
    _ticker.setPeriod(10000/_obj->getSpeed());
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
    _leds[pos] += CHSV( beat8(_obj->getSpeed()/3) + random8(64), 200, 255);
    show();
}

void StarPattern::init()
{
    _stars = new uint8_t[_nbLed];
    std::fill(_stars, _stars+_nbLed, 0);

    fill_solid(_leds,_nbLed, CRGB::Black); 
    show();
}
void StarPattern::display()
{

    for(uint16_t i=0; i<_nbLed; i++){
        if (_stars[i]==0 && random16(60*_obj->getSpeed())==0){
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

void PridePattern::display()
{
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
  show();
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

CRGB TwinklePattern::makeBrighter( const CRGB& color, fract8 howMuchBrighter)
{
  CRGB incrementalColor = color;
  incrementalColor.nscale8( howMuchBrighter);
  return color + incrementalColor;
}
CRGB TwinklePattern::makeDarker( const CRGB& color, fract8 howMuchDarker)
{
  CRGB newcolor = color;
  newcolor.nscale8( 255 - howMuchDarker);
  return newcolor;
}
void TwinklePattern::init()
{
    _ticker.reset();
    _ticker.setPeriod(40);  
    _directionFlags = new uint8_t[(_nbLed+7)/8];
    show();
}
bool TwinklePattern::getPixelDirection( uint16_t i)
{
  uint16_t index = i / 8;
  uint8_t  bitNum = i & 0x07;

  uint8_t  andMask = 1 << bitNum;
  return (_directionFlags[index] & andMask) != 0;
}
void TwinklePattern::setPixelDirection( uint16_t i, bool dir)
{
  uint16_t index = i / 8;
  uint8_t  bitNum = i & 0x07;

  uint8_t  orMask = 1 << bitNum;
  uint8_t andMask = 255 - orMask;
  uint8_t value = _directionFlags[index] & andMask;
  if ( dir ) {
    value += orMask;
  }
  _directionFlags[index] = value;
}
void TwinklePattern::brightenOrDarkenEachPixel(fract8 fadeUpAmount, fract8 fadeDownAmount)
{
   for(uint16_t i=0; i<_nbLed; i++) {
    if ( getPixelDirection(i) == GETTING_DARKER) {
      // This pixel is getting darker
      _leds[i] = makeDarker( _leds[i], fadeDownAmount);
    } else {
      // This pixel is getting brighter
      _leds[i] = makeBrighter( _leds[i], fadeUpAmount);
      // now check to see if we've maxxed out the brightness
      if ( _leds[i].r == 255 || _leds[i].g == 255 || _leds[i].b == 255) {
        // if so, turn around and start getting darker
        setPixelDirection(i, GETTING_DARKER);
      }
    }
  }
}
void TwinklePattern::display()
{
  if(_ticker){
    // Make each pixel brighter or darker, depending on
    // its 'direction' flag.
    brightenOrDarkenEachPixel(_fadeInSpeed, _fadeOutSpeed);
    // Now consider adding a new random twinkle
    if ( random8() < _density ) {
      int pos =random16(_nbLed);
      if ( !_leds[pos]) {
        _leds[pos] = ColorFromPalette( gCurrentPalette, random8(), _startingBritghtness, NOBLEND);
        setPixelDirection(pos, GETTING_BRIGHTER);
      }
    }
    show();
  }
}

void HeatMapPattern::init(){
    _halfLedCount = _nbLed / 2;
    _heat = new byte*[2];
         _heat[0] = new byte[_nbLed/2];
         _heat[1] = new byte[_nbLed/2];

    show();
}
void HeatMapPattern::display()
{
  fill_solid(_leds,_nbLed, CRGB::Black); 

  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(256));

  byte colorindex;

  for (uint8_t x = 0; x < 2; x++) {
    // Step 1.  Cool down every cell a little
    for ( int i = 0; i < _halfLedCount; i++) {
      _heat[x][i] = qsub8( _heat[x][i],  random8(0, ((_cooling * 10) / _halfLedCount) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for ( int k = _halfLedCount - 1; k >= 2; k--) {
      _heat[x][k] = (_heat[x][k - 1] + _heat[x][k - 2] + _heat[x][k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if ( random8() < _sparking ) {
      int y = random8(7);
      _heat[x][y] = qadd8( _heat[x][y], random8(160, 255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for ( int j = 0; j < _nbLed / 2; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      colorindex = scale8(_heat[x][j], 240);

      CRGB color = ColorFromPalette(gCurrentPalette, colorindex);

      if (_up) {
        if (x == 0) {
          _leds[(_halfLedCount - 1) - j] = color;
        }
        else {
          _leds[_halfLedCount + j] = color;
        }
      }
      else {
        if (x == 0) {
          _leds[j] = color;
        }
        else {
          _leds[(_nbLed - 1) - j] = color;
        }
      }
    }
  }
  show();
}

void ColorWavePattern::display()
{

  // uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < _nbLed; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if ( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    //index = triwave8( index);
    index = scale8( index, 240);

    CRGB newcolor = ColorFromPalette(gCurrentPalette, index, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (_nbLed - 1) - pixelnumber;

    nblend(_leds[pixelnumber], newcolor, 128);
  }
  show();
}

void SinelonPattern::display()
{
// a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(_leds, _nbLed,20);
  int pos = beatsin16(_obj->getSpeed()/2,0,_nbLed-1);
  if ( pos < prevpos ) {
    fill_solid( _leds + pos, (prevpos - pos) + 1, CHSV(beat8(_obj->getSpeed()/3), 220, 255));
  } else {
    fill_solid( _leds + prevpos, (pos - prevpos) + 1, CHSV( beat8(_obj->getSpeed()/3), 220, 255));
  }
  prevpos = pos;
  show();
}



