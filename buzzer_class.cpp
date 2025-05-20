#ifndef _BUZZER_CLASS_CPP_
#define _BUZZER_CLASS_CPP_

#include "mbed.h"

class Buzzer {
private:
    PwmOut _buzzer;

    enum class Note {
        Note_c = 0,
        Note_d,
        Note_e,
        Note_f,
        Note_g,
        Note_a,
        Note_b,
        Note_C
    };

    int note_periods[8] = {
        3830, // C (도)
        3400, // D (레)
        3038, // E (미)
        2864, // F (파)
        2550, // G (솔)
        2272, // A (라)
        2028, // B (시)
        1912  // C' (높은 도)
    };

    void playSound(Note note, int duration_ms) {
        int period_us = note_periods[static_cast<int>(note)];
        _buzzer.period_us(period_us);
        _buzzer = 0.5;

        std::chrono::milliseconds duration(duration_ms);
        ThisThread::sleep_for(duration);

        _buzzer = 0;
    }

public:
    Buzzer(PinName pin) : _buzzer(pin) {
        _buzzer = 0;
    }

    // 문 열림: 미 솔 도
    void doorOpen() {
        playSound(Note::Note_e, 100);
        ThisThread::sleep_for(50ms);
        playSound(Note::Note_g, 100);
        ThisThread::sleep_for(50ms);
        playSound(Note::Note_C, 100);
    }

    // 문 닫힘: 도 솔 미
    void doorClose() {
        playSound(Note::Note_C, 100);
        ThisThread::sleep_for(50ms);
        playSound(Note::Note_g, 100);
        ThisThread::sleep_for(50ms);
        playSound(Note::Note_e, 100);
    }

    // 비밀번호 입력: 라
    void passwordInput() {
        playSound(Note::Note_a, 150);
    }

    // 비밀번호 입력 > 일반 모드 : 레
    void modeChangeNormal() {
        playSound(Note::Note_d, 400);
    }

    // 일반 > 비밀번호 입력 모드 : 시
    void modeChangeInputl() {
        playSound(Note::Note_b, 400);
    }

    // 입력 실패: 도 도
    void inputFailed() {
        playSound(Note::Note_c, 100);
        ThisThread::sleep_for(100ms);
        playSound(Note::Note_c, 100);
    }
};

#endif