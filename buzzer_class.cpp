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
    void play(Note note, int duration_ms) {
        int period = _notePeriods[static_cast<int>(note)];
        _buzzer.period_us(period);
        _buzzer = 0.5;

        printf("[BUZZER] Note %d 재생 (%.2fHz, %dms)\r\n", static_cast<int>(note), 1e6f / period, duration_ms);
        ThisThread::sleep_for(std::chrono::milliseconds(duration_ms));

        _buzzer = 0;
    }
};

#endif
