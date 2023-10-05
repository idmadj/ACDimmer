# ESP8266 ACDimmer
An ESP8266 AC dimming implementation compatible with the RoboDyn line of TRIAC-based leading-edge dimmers.

## Features
 - Separate state (on/off) and power level;
 - Basic filtering for dirty zero-crossing signal;
 - Correctly handles minimum and maximum values;
 - Easy power level mapping to only dim between minimum and maximum values;
 - Instanciable interface allowing an "infinite" amount of channels;
 - Customizable utility frequency;
 - Customizable delay between ZC rise and effective ZC;
 - 255 effective power levels.

## Usage
```cpp
#include <ACDimmer.h>

// Initialize a dimmer on pin 4, with a minimum power of 32 and a maximum 
// power of 195 (out of 255)
//
// The power min and max params are optional; defaults: 0, 255
ACDimmer dimmer(4, 32, 195);

void setup () {
    // Setup pin 5 as the ZC pin, with a 100µs delay
    //
    // This implementation only supports one ZC pin, delay and utility 
    // frequency. Any call to configZC applies to all dimmers. Once the first 
    // dimmer is setup, the configuration is locked-in and any subsequent call 
    // to configZC will have no effect.
    //
    // The delay and utility frequency params are optional; defaults: 0, 60
    ACDimmer::configZC(5, 100, 60);

    // Setup the dimmer.
    dimmer.setup();
}

void loop () {
    // Set the dimmer power to 63 (out of 255)
    //
    // Power is mapped (not just clipped) between the minimum and maximum 
    // levels specified during initialization. If min power is 32 and max is 
    // 195, setting power to 63 will effectively turn on power for ~28% of the 
    // AC wave's half-period.
    dimmer.setPower(63);

    // Get the current power level
    uint8_t power = dimmer.getPower();

    // Turn off the dimmer
    dimmer.setState(false);

    // Turn on the dimmer (to the previous power level)
    dimmer.setState(true);

    // Get the current state
    bool state = dimmer.getState();
}
```

## Inspired by
    https://github.com/RobotDynOfficial/RBDDimmer
    https://github.com/sascha432/trailing_edge_dimmer
    https://github.com/Theb-1/ESP8266-wifi-light-dimmer
    https://github.com/AJMansfield/TriacDimmer
    https://github.com/fabianoriccardi/dimmable-light
    
## Additional information
    https://github.com/esphome/issues/issues/1632
    https://github.com/fabianoriccardi/dimmable-light/wiki/Notes-about-dimmer-boards
    https://www.visualmicro.com/page/Timer-Interrupts-Explained.aspx
    https://blocnote360.wordpress.com/2022/03/20/routeur-solaire/
    https://lurchi.wordpress.com/2016/06/29/esp8266-pwm-revisited-and-reimplemented/
