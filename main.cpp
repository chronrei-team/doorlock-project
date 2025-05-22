#include "mbed.h"
#include "push_button.cpp"
#include "joystick.cpp"
#include "motor.cpp"
#include "password_manager.cpp"
#include "buzzer_class.cpp"
#include "DHT22.h"
#include "oled.cpp"
#include "buzzer_class.cpp"
#include <cstdio>

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

// buzzer
#define BUZZER_PIN      PC_9
Buzzer buzzer(BUZZER_PIN);

// DHT22
#define DHT22_DATA_PIN  PB_2
DHT22 dht22(DHT22_DATA_PIN);

//-----------------------------------------------


enum class DoorlockState {
    Open, // 열림
    Close, // 닫힘
    InputOnClose, // 비밀번호 입력
    PasswordFail, // 비밀번호 실패
    OpenAction, // 열리는 중
    CloseAction, // 닫히는 중
};

EventQueue event(EVENTS_EVENT_SIZE * 32);
Timer motorTimer;
Timer autoCloseTimer;

PasswordManager passwordManager;
DoorlockState doorlockState = DoorlockState::Close;

void playSounds(const Note* notes, const int* durations, size_t count, size_t index = 0) {
    if (index >= count) return;

    // 현재 음 재생
    event.call([=] {
        buzzer.play(notes[index], durations[index]);
    });

    // 다음 음 재생 예약 (음 길이 + 간격 고려)
    int delay = durations[index] + 50; // 50ms 간격
    event.call_in(std::chrono::milliseconds(delay), [=] {
        playSounds(notes, durations, count, index + 1);
    });
}

void playSingleSound(Note note, int duration) {
    playSounds(&note, &duration, 1);
}

// oled
DoorlockOled oled(&event);

bool doorlookActionCompleted() {
    printf("모터 시간 : %dms\r\n", (int)(motorTimer.elapsed_time().count() / 1000));
    return motorTimer.elapsed_time() > DOORLOCK_DURATION - 0.1s;
}

void doorlockClose() {
    printf("도어락 닫힘\r\n");
    doorlockState = DoorlockState::CloseAction;
    motor.forward();
    motorTimer.reset();
    autoCloseTimer.reset();
    oled.closingDisplay();

    // 3초간 정회전 후 멈춤
    event.call_in(DOORLOCK_DURATION, [] {
        if (doorlookActionCompleted()) {
            motor.stop();
            doorlockState = DoorlockState::Close;
        }
    });

    // 닫히는 소리 추가
    buzzer.closeSound(&event);
    
}

void doorlockOpen() {
    printf("도어락 오픈\r\n");

    doorlockState = DoorlockState::OpenAction;
    motor.backward();
    motorTimer.reset();
    oled.openingDisplay();

    // 3초간 역회전 후 멈춤
    event.call_in(DOORLOCK_DURATION, [] {
        printf("3초뒤 닫힘");

        if (doorlookActionCompleted()) {
            motor.stop();
            doorlockState = DoorlockState::Open;
        }
    });

    // 열리는 소리 추가
    buzzer.openSound(&event);

    printf("모터 동작 여부");
    
    // 30초뒤 자동 닫힘
    autoCloseTimer.reset();
    event.call_in(AUTO_CLOSE_DURATION, [] {
        if (autoCloseTimer.elapsed_time() > AUTO_CLOSE_DURATION - 0.1s) {
            printf("30초 경과\r\n");
            doorlockClose();
        }
    });
}

void doorlockSliderOpen() {
    doorlockState = DoorlockState::InputOnClose;
    passwordManager.resetInput();
    passwordManager.resetCursor();
    oled.passwordDisplay(passwordManager.getInput(), passwordManager.getCursor(), true, false);
    printf("슬라이더 오픈\r\n");
    // 부저
    buzzer.play(Note::Note_c, 300, &event);
    // oled 제어
}

void doorlockSliderClose() {
    doorlockState = DoorlockState::Close;
    passwordManager.resetInput();
    passwordManager.resetCursor();
    printf("슬라이더 클로즈\r\n");
    // 부저
    buzzer.play(Note::Note_c, 300, &event);
    // oled 제어
}

void authorization() {
    // 패스워드 일치
    if (passwordManager.authorization()) {
        passwordManager.resetCursor();
        passwordManager.resetInput();
        doorlockOpen();
        
        // 부저 소리 출력
        buzzer.passSuccSound(&event);
        // oled 제어
    }
    // 패스워드 불일치
    else {
        doorlockState = DoorlockState::PasswordFail;

        // 부저 소리 출력
        buzzer.passFailSound(&event);
        // oled 제어
        auto aniDelay = oled.passwordFailDisplay(passwordManager.getInput());
        event.call_in(aniDelay, doorlockSliderOpen);
    }
}

void cursorLeft() {
    int cursor = passwordManager.cursorLeft();
    printf("cursor: %d   pw: %d\r\n", cursor, passwordManager.getInput());
    // 단일음 출력
    buzzer.play(Note::Note_e, 300, &event);
    // oled
    oled.passwordDisplay(passwordManager.getInput(), cursor);
}

void cursorRight() {
    int cursor = passwordManager.cursorRight();
    printf("cursor: %d   pw: %d\r\n", cursor, passwordManager.getInput());

    // 단일음 출력
    buzzer.play(Note::Note_e, 300, &event);
    // oled
    oled.passwordDisplay(passwordManager.getInput(), cursor);
}

void inputPlus() {
    int pw = passwordManager.inputPlus();
    printf("cursor: %d   pw: %d\r\n", passwordManager.getCursor(), pw);

    // 단일음 출력
    buzzer.play(Note::Note_g, 300, &event);
    // oled
    oled.passwordDisplay(pw, passwordManager.getCursor(), true, false);
}

void inputMinus() {
    int pw = passwordManager.inputMinus();
    printf("cursor: %d   pw: %d\r\n", passwordManager.getCursor(), pw);

    // 단일음 출력
    buzzer.play(Note::Note_g, 300, &event);
    // oled
    oled.passwordDisplay(pw, passwordManager.getCursor(), true, false);
}

void getTempHumid() {
    if (dht22.sample()) {
        float temperature = (float)dht22.getTemperature() / 10.0f; // 소수점 보정
        float humidity = (float)dht22.getHumidity() / 10.0f;
        printf("[DHT22] 온도: %.1f°C, 습도: %.1f%%\r\n", temperature, humidity);
    } else {
        printf("[DHT22] 센서 측정 실패\r\n");
    }
}

void defaultDisplay() {
    if (doorlockState == DoorlockState::Close) {
        oled.defaultDisplay(true, 25.5, 70);
    }
    else if (doorlockState == DoorlockState::Open) {
        oled.defaultDisplay(false, 25.5, 70);
    }
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

    event.call_every(1s, getTempHumid);
    // sensor init delay
    ThisThread::sleep_for(1s);
}


void debug(JSEdge joystickMovement) {
    if (firstBtn.fallingEdgeTriggered()) {
        printf("첫 번째 버튼 눌러짐\r\n");
        
    } 
    if (secondBtn.fallingEdgeTriggered()) {
        printf("두 번째 버튼 눌러짐\r\n");
        
    } 
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
            defaultDisplay();
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
            defaultDisplay();
            // 세 번째 버튼 누르면 수동 닫기
            if (thirdBtnEdgeTriggered) doorlockClose();
        }
        
        fflush(stdout);
        ThisThread::sleep_for(100ms);
    }
}

