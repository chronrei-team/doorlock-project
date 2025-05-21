#ifndef _JOYSTICK_
#define _JOYSTICK_

#include "mbed.h"

namespace {
    #define JS_CENTER_LOWER 48
    #define JS_CENTER_UPPER 58
}

enum class JSLoc {
    Left = 1,
    Right = 2,
    Up = 3,
    Down = 4,
    Center = 5
};

struct JSEdge {
    JSLoc LeftRight;
    JSLoc UpDown;
};

class JoyStick {
private:
    AnalogIn jsX;
    AnalogIn jsY;
    JSLoc X = JSLoc::Center;
    JSLoc Y = JSLoc::Center;
    JSLoc leftRightPrev = JSLoc::Center;
    JSLoc upDownPrev = JSLoc::Center;
    JSLoc leftRightEdge = JSLoc::Center;
    JSLoc upDownEdge = JSLoc::Center;

    bool isLeft() {
        return jsX * 100 < JS_CENTER_LOWER;
    }

    bool isRight() {
        return jsX * 100 > JS_CENTER_UPPER;
    }

    bool isUp() {
        return jsY * 100 < JS_CENTER_LOWER;
    }

    bool isDown() {
        return jsY * 100 > JS_CENTER_UPPER;
    }

    void edgeSetting() {
        if (leftRightPrev != X) leftRightEdge = X;
        if (upDownPrev != Y) upDownEdge = Y;
        leftRightPrev = X;
        upDownPrev = Y;
    }

public:
    JoyStick(PinName jsXPin, PinName jsYPin) : jsX(jsXPin), jsY(jsYPin) {}

    JSEdge lastTriggeredEdge() {
        JSEdge edge = { leftRightEdge, upDownEdge };
        leftRightEdge = JSLoc::Center;
        upDownEdge = JSLoc::Center;
        return edge;
    }

    void detectLocation() {
        if (isLeft()) X = JSLoc::Left;
        else if (isRight()) X = JSLoc::Right;
        else X = JSLoc::Center;

        if (isUp()) Y = JSLoc::Up;
        else if (isDown()) Y = JSLoc::Down;
        else Y = JSLoc::Center;

        edgeSetting();
    }
};

#endif