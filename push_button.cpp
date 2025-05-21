#ifndef _PUSH_BUTTON_
#define _PUSH_BUTTON_

#include "mbed.h"

class PushButton {
private:
    const char ACTIVE;
    bool isbeforeOff = true;
    bool fallingEdgeEventOn = false;
    DigitalIn button;
    
public:
    PushButton(PinName pin, char active) : button(pin), ACTIVE(active) { }

    bool fallingEdgeTriggered () {
        if (fallingEdgeEventOn) {
            fallingEdgeEventOn = false;
            return true;
        }
        return false;
    };

    void detectFallingEdge() {
        if (isbeforeOff && button == ACTIVE) {
            fallingEdgeEventOn = true;
        }
        isbeforeOff = button != ACTIVE;
    }
};


#endif