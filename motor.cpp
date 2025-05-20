#ifndef _MOTOR_
#define _MOTOR_

#include "mbed.h"

namespace {
    enum MotorState {
        FORWARD = 0,
        BACKWARD = 1,
        Stop = 2,
    };
}

class Motor {
private:
    PwmOut pwm;
    DigitalOut rotate;
    MotorState state = MotorState::Stop;
    float power = 0.5;
public:
    Motor(PinName _pwm, PinName _rotate) : pwm(_pwm), rotate(_rotate) {
        pwm = 0;
        rotate = MotorState::FORWARD;
    }

    void forward() {
        if (state == MotorState::BACKWARD) {
            pwm = 0;
            ThisThread::sleep_for(100ms);
        }

        rotate = MotorState::FORWARD;
        state = MotorState::FORWARD;
        pwm = power;
    }

    void backward() {
        if (state == MotorState::FORWARD) {
            pwm = 0;
            ThisThread::sleep_for(100ms);
        }

        rotate = MotorState::BACKWARD;
        state = MotorState::BACKWARD;
        pwm = power;
    }

    void stop() {
        pwm = 0;
        state = MotorState::Stop;
    }
};

#endif