#pragma once

#include "Led.hpp"

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
            void addGlitter(fract8 chanceOfGlitter);
            uint8_t _nbLed;
            CRGB* _leds;
            const LedObject* _obj;
            bool _show;
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
        TwinkleFoxPattern(const LedObject* obj):LedPattern(obj),_speed(4),_density(5),gBackgroundColor(CRGB::Black),_autoBGColor(false),_coolLikeIncandescent(true){}
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
    class RainbowSoundPattern : public LedPattern  {
        friend LedObject;
        public:
        RainbowSoundPattern(const LedObject* obj):LedPattern(obj){}
        protected:
            void display() override;
    };


} // namespace SmartIotInternals