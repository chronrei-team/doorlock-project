#ifndef _BUZZER_CLASS_CPP_
#define _BUZZER_CLASS_CPP_

#include "mbed.h"

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

class Buzzer {
private:
    PwmOut _buzzer;

    static constexpr int _notePeriods[8] = {
        3830, 3400, 3038, 2864, 2550, 2272, 2028, 1912
    };

public:
    Buzzer(PinName pin) : _buzzer(pin) {
        _buzzer = 0;
    }

    // 단일 음 재생
    void play(Note note, int duration_ms, EventQueue* event) {
        printf("부저음 시작");
        int period = _notePeriods[static_cast<int>(note)];
        _buzzer.period_us(period);
        _buzzer = 0.5;

        printf("[BUZZER] Note %d 재생 (%.2fHz, %dms)\r\n", static_cast<int>(note), 1e6f / period, duration_ms);
        
        event->call_in(std::chrono::milliseconds(duration_ms), [this] {

            _buzzer = 0;
            printf("부저음 종료");
        });
    }

    void openSound(EventQueue* event, size_t index = 0) {
        Note openMelody[] = { Note::Note_C, Note::Note_a, Note::Note_f };
        if (index >= 3) return;
        play(openMelody[index], 300, event);
        event->call_in(std::chrono::milliseconds(350), [=] {
        openSound(event, index + 1);
        });
    }

    void closeSound(EventQueue* event, size_t index = 0) {
        Note closeMelody[] = { Note::Note_e, Note::Note_g, Note::Note_C };
        if (index >= 3) return;
        play(closeMelody[index], 300, event);
        event->call_in(std::chrono::milliseconds(350), [=] {
        closeSound(event, index + 1);
        });
    }

    void passSuccSound(EventQueue* event, size_t index = 0) {
        Note passSuccMelody[] = { Note::Note_g, Note::Note_C};
        if (index >= 2) return;
        play(passSuccMelody[index], 300, event);
        event->call_in(std::chrono::milliseconds(350), [=] {
        passSuccSound(event, index + 1);
        });
    }

    void passFailSound(EventQueue* event, size_t index = 0) {
        Note passFailMelody[] = { Note::Note_f, Note::Note_f};
        if (index >= 2) return;
        play(passFailMelody[index], 300, event);
        event->call_in(std::chrono::milliseconds(350), [=] {
        passFailSound(event, index + 1);
        });
    }
};

#endif
