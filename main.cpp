#include "mbed.h"
#include "push_button.cpp"
#include "joystick.cpp"
#include "motor.cpp"

#define ACTIVE_LOW 0
#define DEBOUNCING_DELAY 50ms
#define DOORLOCK_DURATION 3s


// push button
#define FIRST_BTN_PIN PA_14
#define SECOND_BTN_PIN PB_7
#define THIRD_BTN_PIN PC_4

PushButton firstBtn(FIRST_BTN_PIN, ACTIVE_LOW);
PushButton secondBtn(SECOND_BTN_PIN, ACTIVE_LOW);
PushButton thirdBtn(THIRD_BTN_PIN, ACTIVE_LOW);

// joystick
#define JS_X_PIN PC_2
#define JS_Y_PIN PC_3
JoyStick joyStick(JS_X_PIN, JS_Y_PIN);

// motor
#define MOTOR_A_PWM_PIN PA_7
#define MOTOR_A_ROTATE_PIN PC_8
Motor motor(MOTOR_A_PWM_PIN, MOTOR_A_ROTATE_PIN);

//-----------------------------------------------

char password = 0;

EventQueue event(EVENTS_EVENT_SIZE * 32);
Timer motorTimer;

void doorlookActionComplete() {
    if (motorTimer.elapsed_time() > DOORLOCK_DURATION - 0.1s) {
        motorTimer.stop();
    }
}

void doorlockOpen() {
    motor.backward();
    motorTimer.reset();
    event.call_in(DOORLOCK_DURATION, doorlookActionComplete);

    // 열리는 소리 추가
    
}

void doorlockClose() {
    motor.forward();
    motorTimer.reset();
    event.call_in(DOORLOCK_DURATION, doorlookActionComplete);

    // 닫히는 소리 추가
    
}


void setup() {
    static Thread eventWorker;
    eventWorker.start(callback(&event, &EventQueue::dispatch_forever));

    motorTimer.start();

    // record button falling edge
    event.call_every(DEBOUNCING_DELAY, callback(&firstBtn, &PushButton::detectFallingEdge));
    event.call_every(DEBOUNCING_DELAY, callback(&secondBtn, &PushButton::detectFallingEdge));
    event.call_every(DEBOUNCING_DELAY, callback(&thirdBtn, &PushButton::detectFallingEdge));

    // record joystick movement
    event.call_every(DEBOUNCING_DELAY, callback(&joyStick, &JoyStick::detectLocation));

}


void debug(JSEdge joystickMovement) {
    if (firstBtn.fallingEdgeTriggered()) printf("첫 번째 버튼 눌러짐\r\n");
    if (secondBtn.fallingEdgeTriggered()) printf("두 번째 버튼 눌러짐\r\n");
    if (thirdBtn.fallingEdgeTriggered()) printf("세 번째 버튼 눌러짐\r\n");

    if (joystickMovement.LeftRight == JSLoc::Left) printf("조이스틱 왼쪽 움직임\r\n");
    if (joystickMovement.LeftRight == JSLoc::Right) printf("조이스틱 오른쪽 움직임\r\n");
    if (joystickMovement.UpDown == JSLoc::Up) printf("조이스틱 위로 움직임\r\n");
    if (joystickMovement.UpDown == JSLoc::Down) printf("조이스틱 아래로 움직임\r\n");
}

// main() runs in its own thread in the OS
int main()
{
    while (true) {
        auto joystickMovement = joyStick.lastTriggeredEdge();

        debug(joystickMovement);
    }
}

