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
} // namespace SmartIotInternals