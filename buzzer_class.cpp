#ifndef _BUZZER_CLASS_CPP_
#define _BUZZER_CLASS_CPP_

#include "mbed.h"

class Buzzer {
private:
    PwmOut _buzzer;
    Thread _buzzerThread;
    EventQueue _queue {64}; // _queue(EVENTS_EVENT_SIZE * 64)와 같음.

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

    void stop() {
        _buzzer = 0;
    }

    void playSound(Note note, int duration_ms) {
        int period_us = note_periods[static_cast<int>(note)];
        _buzzer.period_us(period_us);
        _buzzer = 0.5;

        // 이벤트 큐로 부저 끄기
        _queue.call_in(std::chrono::milliseconds(duration_ms), callback(this, &Buzzer::stop));
    }

public:
    Buzzer(PinName pin) : _buzzer(pin) {
        _buzzer = 0;
        _buzzerThread.start(callback(&_queue, &EventQueue::dispatch_forever));
    }

    // 매개변수 : 음, 동작 시간, 지연 시간
    void playSoundAsync(Note note, int duration_ms, int delay_ms) {
        _queue.call_in(std::chrono::milliseconds(delay_ms), callback(this, &Buzzer::playSound), note, duration_ms);
    }

    // 문 열림: 미 솔 도
    void doorOpen() {
        playSoundAsync(Note::Note_e, 100, 0);
        playSoundAsync(Note::Note_g, 100, 150);
        playSoundAsync(Note::Note_C, 100, 300);
    }

    // 문 닫힘: 도 솔 미
    void doorClose() {
        playSoundAsync(Note::Note_C, 100, 0);
        playSoundAsync(Note::Note_g, 100, 150);
        playSoundAsync(Note::Note_e, 100, 300);
    }

    // 비밀번호 입력: 라
    void passwordInput() {
        playSoundAsync(Note::Note_a, 150, 0);
    }

    // 비밀번호 입력 > 일반 모드 : 레
    void modeChangeNormal() {
        playSoundAsync(Note::Note_d, 400, 0);
    }

    // 일반 > 비밀번호 입력 모드 : 시
    void modeChangeInput() {
        playSoundAsync(Note::Note_b, 400, 0);
    }

    // 입력 실패: 도 도
    void inputFailed() {
        playSoundAsync(Note::Note_c, 100, 0);
        playSoundAsync(Note::Note_c, 100, 150);
    }
};

#endif