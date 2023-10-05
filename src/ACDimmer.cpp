#include "ACDimmer.h"
#include <limits.h>

using namespace std;

uint8_t ACDimmer::zcPin = -1;
uint16_t ACDimmer::zcDelay = 0;
uint8_t ACDimmer::utilityFrequency = 60;
const float ACDimmer::HALF_PERIOD = (((1.0f / float(utilityFrequency)) / 2.0f) * 1000000.0f);
const uint8_t ACDimmer::POWER_LEVELS = -1;
bool ACDimmer::setupDone = false;
volatile unsigned long ACDimmer::zcRise;
volatile bool ACDimmer::zcRiseInitialized = false;
volatile unsigned long ACDimmer::zcTime;
volatile bool ACDimmer::zcTimeInitialized = false;
forward_list<ACDimmerDevice*> ACDimmer::devices;

ACDimmer::ACDimmer(
    const uint8_t _PIN, 
    const uint8_t _POWER_MIN, 
    const uint8_t _POWER_MAX
) : 
device{_PIN},
POWER_MIN(_POWER_MIN),
POWER_MAX(_POWER_MAX)
{}

void ACDimmer::setup() {
    setupSingle();
    pinMode(device.PIN, OUTPUT);
    devices.push_front(&device);
}

void ACDimmer::applyDiscrete() {
    if (device.state) {
        if (device.power == 0) {
            digitalWrite(device.PIN, LOW);
        } else if (device.power == POWER_LEVELS) {
            digitalWrite(device.PIN, HIGH);
        }
        // All other cases are dealt with by the timer interrupt.
    } else {
        digitalWrite(device.PIN, LOW);
    }
}

void ACDimmer::setPower(uint8_t _power) {
    uint8_t constrainedPower = constrain(_power, 0, POWER_LEVELS);

    device.power = constrainedPower;
    device.mappedPower = mapValue(constrainedPower, 0, POWER_LEVELS, POWER_MIN, POWER_MAX);

    applyDiscrete();
}

uint8_t ACDimmer::getPower() {
    return device.power;
}

void ACDimmer::setState(bool _state) {
    device.state = _state;

    applyDiscrete();
}

bool ACDimmer::getState() {
    return device.state;
}

void ACDimmer::configZC(uint8_t _pin, uint16_t _delay, uint8_t _utilityFrequency) {
    if (_pin != uint8_t(-1)) {
        zcPin = _pin;
    }
    
    if (_delay != uint16_t(-1)) {
        zcDelay = _delay;
    }

    if (_utilityFrequency != uint8_t(-1)) {
        utilityFrequency = _utilityFrequency;
    }
}

void ACDimmer::setupSingle() {
    if (!setupDone && zcPin != uint8_t(-1)) {
        pinMode(zcPin, INPUT_PULLUP);
        attachInterrupt(zcPin, onZCPinRising, RISING);

        timer1_attachInterrupt(onTimerInterrupt);
        timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
    	// half-period divided in 255 subunits (one per power level), times 5 since we're using TIM_DIV16.
        timer1_write(((HALF_PERIOD / float(POWER_LEVELS)) * 5.0f));

        setupDone = true;
    }
}

float ACDimmer::mapValue(float value, float fromMin, float fromMax, float toMin, float toMax) {
    return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
}

void ICACHE_RAM_ATTR ACDimmer::onZCPinRising() {
    if ((micros() - zcRise) > (HALF_PERIOD / 2)) { // Debouncing in case of dirty signal
        zcRise = micros();
        zcRiseInitialized = true;
    }
}

void ICACHE_RAM_ATTR ACDimmer::onTimerInterrupt() {
    if (zcRiseInitialized) {
        unsigned long currentTime = micros();

        // zcNext is the point in time at which we assume the real zero-crossing is happening. 
        unsigned long zcNext = zcRise + zcDelay;

        // zcNext might be in the future, so we only use it to determine the current progress into the 
        // half period if it is before the current time, otherwise we keep using the previously defined 
        // value for zcTime.
        if ((currentTime - zcNext) < (ULONG_MAX/2)) { // Takes overflow into account
            zcTime = zcNext;
            zcTimeInitialized = true;
        }

        if (zcTimeInitialized) {
            float progress = float(currentTime - zcTime) / HALF_PERIOD;
            float constrainedProgress = constrain(progress, 0.0f, 1.0f);

            for(ACDimmerDevice* device : devices) {
                uint8_t power = device->power;

                if (device->state && power != 0 && power != POWER_LEVELS) {
                    if ((1.0f - constrainedProgress) < (device->mappedPower / float(POWER_LEVELS))) {
                        digitalWrite(device->PIN, HIGH);
                    } else {
                        digitalWrite(device->PIN, LOW);
                    }
                }
            }
        }
    }
}
