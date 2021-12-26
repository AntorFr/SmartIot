#pragma once

#include "Led.hpp"
#include "GradientPalettes.hpp"

class LedObject;

namespace SmartIotInternals {
    class LedPattern {
        friend LedObject;

        public:
        LedPattern(const LedObject*  obj);
        bool toShow() {return _show;}
        void show() {_show=true;}
        void showed() {_show=false;}

        protected:
            virtual void init() {};
            virtual void display() {};
            void addGlitter(fract8 chanceOfGlitter) {_chanceOfGlitter = chanceOfGlitter;}
            void addPowerCut(fract16 chanceOfPowercut){_chanceOfPowercut = _chanceOfPowercut;}
            uint8_t _nbLed;
            CRGB* _leds;
            const LedObject* _obj;
            bool _show;

            fract8 _chanceOfGlitter;
            fract16 _chanceOfPowercut;
        
        private:
            uint8_t _pwrCutMem;
            void glitter();
            void powerCut();
    };
    class OffPattern : public LedPattern  {
        friend LedObject;
        public:
        OffPattern(const LedObject* obj):LedPattern(obj){}
        protected:
            void init() override;
    };
    class ColorPattern : public LedPattern  {
        friend LedObject;

        public:
        ColorPattern(const LedObject* obj):LedPattern(obj){}

        protected:
            void init() override;

    };
    class BlinkPattern : public LedPattern  {
        friend LedObject;
        public:
        BlinkPattern(const LedObject* obj):LedPattern(obj),_blink(false){}

        protected:
            void init() override;
            void display() override;
        private:
        CEveryNMillis _ticker;
        bool _blink;
    };
    class WipePattern : public LedPattern  {
        friend LedObject;
        public:
        WipePattern(const LedObject* obj):LedPattern(obj){}
        protected:
            void display() override;
    };
    class LaserPattern : public LedPattern  {
        friend LedObject;
        public:
        LaserPattern(const LedObject* obj):LedPattern(obj){}
        protected:
            void display() override;
    };
    class BreathePattern : public LedPattern  {
        friend LedObject;
        public:
        BreathePattern(const LedObject* obj):LedPattern(obj){}
        protected:
            void display() override;
    };
    class RainbowPattern : public LedPattern  {
        friend LedObject;
        public:
        RainbowPattern(const LedObject* obj):LedPattern(obj){}
        protected:
            void display() override;
    };
    class K2000Pattern : public LedPattern  {
        friend LedObject;
        public:
        K2000Pattern(const LedObject* obj):LedPattern(obj){}
        protected:
            void display() override;
    };
    class ComputerPattern : public LedPattern  {
        friend LedObject;
        public:
        ComputerPattern(const LedObject* obj):LedPattern(obj){}
        protected:
            void display() override;
    };
    class ConfettiPattern : public LedPattern  {
        friend LedObject;
        public:
        ConfettiPattern(const LedObject* obj):LedPattern(obj){}
        protected:
            void display() override;
    };
    class StarPattern : public LedPattern  {
        friend LedObject;
        public:
        StarPattern(const LedObject* obj):LedPattern(obj),_stars(nullptr){}
        protected:
            uint8_t* _stars;
            void init() override;
            void display() override;
    };
    class PridePattern : public LedPattern  {
        friend LedObject;
        public:
        PridePattern(const LedObject* obj):LedPattern(obj),sPseudotime(0),sLastMillis(0),sHue16(0){}
        protected:
            void display() override;
        private:
            uint16_t sPseudotime;
            uint16_t sLastMillis;
            uint16_t sHue16 ;
    };
    class TwinkleFoxPattern : public LedPattern  {
        friend LedObject;
        public:
            TwinkleFoxPattern(const LedObject* obj, CRGBPalette16 palette):LedPattern(obj),_speed(4),_density(5),gBackgroundColor(CRGB::Black),_autoBGColor(false),_coolLikeIncandescent(true){gCurrentPalette = palette;}
        protected:
            void display() override;
            CRGB computeOneTwinkle( uint32_t ms, uint8_t salt);
            void coolLikeIncandescent( CRGB& c, uint8_t phase);
            uint8_t attackDecayWave8( uint8_t i);
        private:
            uint8_t _speed;
            uint8_t _density;
            CRGB gBackgroundColor;
            bool _autoBGColor;
            bool _coolLikeIncandescent;
            CRGBPalette16 gCurrentPalette;
    };
    class TwinklePattern : public LedPattern  {
        friend LedObject;
        public:
            TwinklePattern(const LedObject* obj, CRGBPalette16 palette):LedPattern(obj),_startingBritghtness(64),_fadeInSpeed(32),_fadeOutSpeed(20),_density(255){ gCurrentPalette = palette;}
        protected:
            void init() override;
            void display() override;
            CRGB makeBrighter( const CRGB& color, fract8 howMuchBrighter);
            CRGB makeDarker( const CRGB& color, fract8 howMuchDarker);
            bool getPixelDirection( uint16_t i);
            void setPixelDirection( uint16_t i, bool dir);
            void brightenOrDarkenEachPixel(fract8 fadeUpAmount, fract8 fadeDownAmount);

        private:
            CEveryNMillis _ticker;
            uint8_t _startingBritghtness;
            uint8_t _fadeInSpeed;
            uint8_t _fadeOutSpeed;
            uint8_t _density;
            uint8_t*  _directionFlags;
            CRGBPalette16 gCurrentPalette;
            enum { GETTING_DARKER = 0, GETTING_BRIGHTER = 1 };
    };
    class RainbowSoundPattern : public LedPattern  {
        friend LedObject;
        public:
        RainbowSoundPattern(const LedObject* obj):LedPattern(obj){}
        protected:
            void display() override;
    };
    class HeatMapPattern : public LedPattern  {
        friend LedObject;
        public:
        HeatMapPattern(const LedObject* obj,CRGBPalette16 palette, bool up):LedPattern(obj),_cooling(49),_sparking(60){gCurrentPalette = palette; _up = up;}
        protected:
            void init() override;
            void display() override;
        private:
            CRGBPalette16 gCurrentPalette;
            bool _up;
            uint8_t _halfLedCount;
            byte** _heat;
            uint8_t _cooling;
            uint8_t _sparking;
    };
    class ColorWavePattern : public LedPattern  {
        friend LedObject;
        public:
        ColorWavePattern(const LedObject* obj,CRGBPalette16 palette):LedPattern(obj),sPseudotime(0),sLastMillis(0),sHue16(0){gCurrentPalette = palette;}
        protected:
            void display() override;
        private:
            CRGBPalette16 gCurrentPalette;
            uint16_t sPseudotime;
            uint16_t sLastMillis;
            uint16_t sHue16;
    };
    class SinelonPattern : public LedPattern  {
        friend LedObject;
        public:
        SinelonPattern(const LedObject* obj):LedPattern(obj),prevpos(0){}
        protected:
            void display() override;
        private:
            uint16_t prevpos;
    };

} // namespace SmartIotInternals