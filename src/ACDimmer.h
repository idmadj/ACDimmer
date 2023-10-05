#ifndef ACDimmer_h
#define ACDimmer_h

#include <stdint.h>
#include "Arduino.h"
#include <forward_list>

using namespace std;

struct ACDimmerDevice {
    const uint8_t PIN;
    uint8_t power;
    float mappedPower;
    bool state;
};

class ACDimmer {
    public:
        ACDimmer(const uint8_t _PIN, const uint8_t _POWER_MIN = 0, const uint8_t _POWER_MAX = -1);

        void setup();
        void setPower(uint8_t _power);
        uint8_t getPower();
        void setState(bool _state);
        bool getState();

        static void configZC(uint8_t _pin = -1, uint16_t _delay = -1, uint8_t _utilityFrequency = -1);

    private:
        ACDimmerDevice device;
        const uint8_t POWER_MIN;
        const uint8_t POWER_MAX;

        void applyDiscrete();

        static uint8_t zcPin;
        static uint16_t zcDelay;
        static uint8_t utilityFrequency;
        static const float HALF_PERIOD;
        static const uint8_t POWER_LEVELS;
        static bool setupDone;
        static volatile unsigned long zcRise;
        static volatile bool zcRiseInitialized;
        static volatile unsigned long zcTime;
        static volatile bool zcTimeInitialized;
        static forward_list<ACDimmerDevice*> devices;

        static void setupSingle();
        static void onTimer(function<void(float)> observer);
        static float mapValue(float value, float fromMin, float fromMax, float toMin, float toMax);
        static void ICACHE_RAM_ATTR onZCPinRising();
        static void ICACHE_RAM_ATTR onTimerInterrupt();
};

#endif
