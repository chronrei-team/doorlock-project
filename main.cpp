#include "mbed.h"
#include "push_button.cpp"
#include "joystick.cpp"
#include "motor.cpp"
#include "password_manager.cpp"

#define ACTIVE_LOW 0
#define DEBOUNCING_DELAY 50ms
#define DOORLOCK_DURATION 3s
#define AUTO_CLOSE_DURATION 30s


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


enum class DoorlockState {
    Open, // 열림
    Close, // 닫힘
    InputOnClose, // 비밀번호 입력
    OpenAction, // 열리는 중
    CloseAction, // 닫히는 중
};

EventQueue event(EVENTS_EVENT_SIZE * 32);
Timer motorTimer;
Timer autoCloseTimer;

PasswordManager passwordManager;
DoorlockState doorlockState = DoorlockState::Close;

bool doorlookActionCompleted() {
    return motorTimer.elapsed_time() > DOORLOCK_DURATION - 0.1s;
}

void doorlockClose() {
    doorlockState = DoorlockState::CloseAction;
    motor.forward();
    motorTimer.reset();

    // 3초간 정회전 후 멈춤
    event.call_in(DOORLOCK_DURATION, [] {
        if (doorlookActionCompleted()) {
            motor.stop();
            doorlockState = DoorlockState::Close;
        }
    });

    // 닫히는 소리 추가
    
}

void doorlockOpen() {
    doorlockState = DoorlockState::OpenAction;
    motor.backward();
    motorTimer.reset();

    // 3초간 역회전 후 멈춤
    event.call_in(DOORLOCK_DURATION, [] {
        if (doorlookActionCompleted()) {
            motor.stop();
            doorlockState = DoorlockState::Open;
        }
    });

    // 열리는 소리 추가
    
    // 30초뒤 자동 닫힘
    autoCloseTimer.reset();
    event.call_in(AUTO_CLOSE_DURATION, [] {
        if (autoCloseTimer.elapsed_time() > AUTO_CLOSE_DURATION - 0.1s) {
            doorlockClose();
        }
    });
}

void doorlockSliderOpen() {
    doorlockState = DoorlockState::InputOnClose;
    passwordManager.resetInput();
    passwordManager.resetCursor();

    // oled 제어
}

void doorlockSliderClose() {
    doorlockState = DoorlockState::Close;
    passwordManager.resetInput();
    passwordManager.resetCursor();

    // oled 제어
}

void authorization() {
    // 패스워드 일치
    if (passwordManager.authorization()) {
        doorlockOpen();
        printf("비밀번호 통과\r\n");
        
        // 부저 소리 출력
        // oled 제어
    }
    // 패스워드 불일치
    else {
        printf("비밀번호 실패\r\n");

        // 부저 소리 출력
        // oled 제어
    }
}

void cursorLeft() {
    int cursor = passwordManager.cursorLeft();
    printf("cursor: %d   pw: %d\r\n", cursor, passwordManager.getInput());
    // 소리
    // oled
}

void cursorRight() {
    int cursor = passwordManager.cursorRight();
    printf("cursor: %d   pw: %d\r\n", cursor, passwordManager.getInput());

    // 소리
    // oled
}

void inputPlus() {
    int pw = passwordManager.inputPlus();
    printf("cursor: %d   pw: %d\r\n", passwordManager.getCursor(), pw);

    // 소리
    // oled
}

void inputMinus() {
    int pw = passwordManager.inputMinus();
    printf("cursor: %d   pw: %d\r\n", passwordManager.getCursor(), pw);

    // 소리
    // oled
}


void setup() {
    static Thread eventWorker;
    eventWorker.start(callback(&event, &EventQueue::dispatch_forever));

    motorTimer.start();
    autoCloseTimer.start();

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

/*
    추가기능으로..
    1. 비밀번호 재설정?
    2. 일정 횟수 틀리면 보안 알람?
*/

// main() runs in its own thread in the OS
int main()
{
    setup();
    while (true) {
        auto joystickMovement = joyStick.lastTriggeredEdge();
        bool firstBtnEdgeTriggered = firstBtn.fallingEdgeTriggered();
        bool secondBtnEdgeTriggered = secondBtn.fallingEdgeTriggered();
        bool thirdBtnEdgeTriggered = thirdBtn.fallingEdgeTriggered();

        if (doorlockState == DoorlockState::Close) {
            // 세 번째 버튼 누르면 수동 열기
            if (thirdBtnEdgeTriggered) doorlockOpen();
            // 두 번째 버튼을 누르면 비밀번호 입력 준비 (도어락의 슬라이더를 위로 올리는 효과)
            else if (secondBtnEdgeTriggered) doorlockSliderOpen();
        }
        else if (doorlockState == DoorlockState::InputOnClose) {
            // 세 번째 버튼 누르면 수동 열기
            if (thirdBtnEdgeTriggered) doorlockOpen();
            // 두 번째 버튼 누르면 비밀번호 입력 취소 (도어락의 슬라이더를 닫는 효과)
            else if (secondBtnEdgeTriggered) doorlockSliderClose();
            // 첫 번째 버튼 누르면 비밀번호 입력 확인
            else if (firstBtnEdgeTriggered) authorization();

            // 패스워드 조작
            if (joystickMovement.LeftRight == JSLoc::Left) cursorLeft();
            else if (joystickMovement.LeftRight == JSLoc::Right) cursorRight();
            else if (joystickMovement.UpDown == JSLoc::Up) inputPlus();
            else if (joystickMovement.UpDown == JSLoc::Down) inputMinus();
        }
        else if (doorlockState == DoorlockState::Open) {
            // 세 번째 버튼 누르면 수동 닫기
            if (thirdBtnEdgeTriggered) doorlockClose();
        }
        
        ThisThread::sleep_for(10ms);
    }
}

