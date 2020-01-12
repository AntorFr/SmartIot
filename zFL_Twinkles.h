// based on ColorTwinkles by Mark Kriegsman: https://gist.github.com/kriegsman/5408ecd397744ba0393e

#define STARTING_BRIGHTNESS 64
#define FADE_IN_SPEED       32
#define FADE_OUT_SPEED      20
#define DENSITY            255

enum { GETTING_DARKER = 0, GETTING_BRIGHTER = 1 };

CRGB makeBrighter( const CRGB& color, fract8 howMuchBrighter)
{
  CRGB incrementalColor = color;
  incrementalColor.nscale8( howMuchBrighter);
  return color + incrementalColor;
}

CRGB makeDarker( const CRGB& color, fract8 howMuchDarker)
{
  CRGB newcolor = color;
  newcolor.nscale8( 255 - howMuchDarker);
  return newcolor;
}

// Compact implementation of
// the directionFlags array, using just one BIT of RAM
// per pixel.  This requires a bunch of bit wrangling,
// but conserves precious RAM.  The cost is a few
// cycles and about 100 bytes of flash program memory.
uint8_t  directionFlags[ (NUM_LEDS + 7) / 8];

bool getPixelDirection( uint16_t i)
{
  uint16_t index = i / 8;
  uint8_t  bitNum = i & 0x07;

  uint8_t  andMask = 1 << bitNum;
  return (directionFlags[index] & andMask) != 0;
}

void setPixelDirection( uint16_t i, bool dir)
{
  uint16_t index = i / 8;
  uint8_t  bitNum = i & 0x07;

  uint8_t  orMask = 1 << bitNum;
  uint8_t andMask = 255 - orMask;
  uint8_t value = directionFlags[index] & andMask;
  if ( dir ) {
    value += orMask;
  }
  directionFlags[index] = value;
}

void brightenOrDarkenEachPixel(led_obj obj, fract8 fadeUpAmount, fract8 fadeDownAmount)
{
  for ( uint16_t i = obj.start_point; i < obj.nb_led; i++) {
    if ( getPixelDirection(i) == GETTING_DARKER) {
      // This pixel is getting darker
      leds[i] = makeDarker( leds[i], fadeDownAmount);
    } else {
      // This pixel is getting brighter
      leds[i] = makeBrighter( leds[i], fadeUpAmount);
      // now check to see if we've maxxed out the brightness
      if ( leds[i].r == 255 || leds[i].g == 255 || leds[i].b == 255) {
        // if so, turn around and start getting darker
        setPixelDirection(i, GETTING_DARKER);
      }
    }
  }
}
void brightenOrDarkenEachPixel(fract8 fadeUpAmount, fract8 fadeDownAmount) { brightenOrDarkenEachPixel((led_obj) {0,NUM_LEDS},fadeUpAmount,fadeDownAmount);}

void colortwinkles(led_obj obj)
{
  EVERY_N_MILLIS(30)
  {
    // Make each pixel brighter or darker, depending on
    // its 'direction' flag.
    brightenOrDarkenEachPixel(obj, FADE_IN_SPEED, FADE_OUT_SPEED);
  
    // Now consider adding a new random twinkle
    if ( random8() < DENSITY ) {
      int pos = obj.start_point + random16(obj.nb_led);
      if ( !leds[pos]) {
        leds[pos] = ColorFromPalette( gCurrentPalette, random8(), STARTING_BRIGHTNESS, NOBLEND);
        setPixelDirection(pos, GETTING_BRIGHTER);
      }
    }
  }
}
void colortwinkles() {colortwinkles((led_obj) {0,NUM_LEDS});}

void cloudTwinkles(led_obj obj)
{
  gCurrentPalette = CloudColors_p; // Blues and whites!
  colortwinkles();
}
void cloudTwinkles() {cloudTwinkles((led_obj) {0,NUM_LEDS});}

void rainbowTwinkles(led_obj obj)
{
  gCurrentPalette = RainbowColors_p;
  colortwinkles();
}
void rainbowTwinkles() {rainbowTwinkles((led_obj) {0,NUM_LEDS});}


void snowTwinkles(led_obj obj)
{
  CRGB w(85, 85, 85), W(CRGB::White);

  gCurrentPalette = CRGBPalette16( W, W, W, W, w, w, w, w, w, w, w, w, w, w, w, w );
  colortwinkles();
}
void snowTwinkles() {snowTwinkles((led_obj) {0,NUM_LEDS});}


void incandescentTwinkles(led_obj obj)
{
  CRGB l(0xE1A024);

  gCurrentPalette = CRGBPalette16( l, l, l, l, l, l, l, l, l, l, l, l, l, l, l, l );
  colortwinkles();
}
void incandescentTwinkles() {incandescentTwinkles((led_obj) {0,NUM_LEDS});}
