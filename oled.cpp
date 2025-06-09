#ifndef _OLED_CPP_
#define _OLED_CPP_

#include "mbed.h"
#include "Adafruit_SSD1306.h"
#include "DoorlockState.h"
#include <chrono>
#include <cstdio>
#include <string>

namespace {
    // an I2C sub-class that provides a constructed default
    class I2CPreInit : public I2C
    {
    public:
        I2CPreInit(PinName sda, PinName scl) : I2C(sda, scl)
        {
            frequency(400000);
    //        start();
        };
    };
}


class DoorlockOled {
private:
    I2CPreInit myI2C;
    Adafruit_SSD1306_I2c oled;

    EventQueue *eventQueue;

    const chrono::milliseconds ani = 500ms;
    Timer aniTimer;

    void passwordDisplaySetting() {
        oled.clearDisplay();

        oled.setTextSize(1);
        oled.setTextCursor(23, 0);
        oled.printf("password input");

        oled.setTextSize(4);
        oled.setTextCursor(19, 16);
    }

    void display() {
        eventQueue->call([this] {
            oled.display();
        });
    }

    void passwordCursorBlack(int password, int cursor) {
        for (int i = 0; i < 4; i++) {
            if (cursor - 1 == i) {
                oled.printf(" ");
            }
            else {
                oled.printf("%d", (password / (int)pow(10, 3 - i)) % 10);
            }
        }
        display();
    }

    void passwordAll(int password) {
        oled.printf("%04d", password);
        display();
    }
    
public:
    DoorlockOled(EventQueue *event) : myI2C(I2C_SDA, I2C_SCL), oled(myI2C, D13, 0x78, 64, 128) { 
        eventQueue = event;
        aniTimer.start();
    }


    void passwordDisplay(int password, int cursor, bool firstCall = true, bool cursorBlack = true) {
        if (doorlockState != DoorlockState::InputOnClose || (!firstCall && aniTimer.elapsed_time() <= ani - 10ms)) return;
        aniTimer.stop();

        passwordDisplaySetting();
        if (cursorBlack) passwordCursorBlack(password, cursor);
        else passwordAll(password);

        aniTimer.reset();
        aniTimer.start();
        eventQueue->call_in(ani, [this, password, cursor, cursorBlack]() {
            if (doorlockState != DoorlockState::InputOnClose || aniTimer.elapsed_time() <= ani - 10ms) return;

            passwordDisplaySetting();
            if (cursorBlack) passwordAll(password);
            else passwordCursorBlack(password, cursor);

            aniTimer.reset();
            
            eventQueue->call_in(ani, callback(this, &DoorlockOled::passwordDisplay),
                password, cursor, false, cursorBlack);
        });
    }

    chrono::milliseconds passwordFailDisplay(int password, int aniCount = 1) {
        if (aniCount == 1) {
            aniTimer.stop();
            aniTimer.reset();
        }

        oled.clearDisplay();
        display();

        if (aniCount == 1) {
            eventQueue->call_in(ani, [this, password, aniCount] {
                passwordDisplaySetting();
                passwordAll(password);
                eventQueue->call_in(ani, callback(this, &DoorlockOled::passwordFailDisplay),
                    password, aniCount + 1);
            });
        }
        
        return ani * 3;
    }

    void closingDisplay() {
        aniTimer.stop();
        aniTimer.reset();
        oled.clearDisplay();
        oled.setTextSize(2);
        oled.setTextCursor(22, 16);
        oled.printf("closing");
        display();
    }

    void openingDisplay() {
        aniTimer.stop();
        aniTimer.reset();
        oled.clearDisplay();
        oled.setTextSize(2);
        oled.setTextCursor(22, 16);
        oled.printf("Opening");
        display();
    }

    void defaultDisplay(bool isClose, float temp, float humi) {
        aniTimer.stop();
        aniTimer.reset();
        oled.clearDisplay();

        oled.setTextSize(1);

        oled.setTextCursor(20, 0);
        oled.printf("Close");

        oled.setTextCursor(68, 0);
        oled.printf("Temp/Humi");

        oled.setTextSize(3);
        if (isClose) {
            oled.setTextCursor(22, 17);
            oled.printf("Ok");
        }
        else {
            oled.setTextCursor(22, 17);
            oled.printf("No");
        }

        oled.setTextCursor(68, 16);
        oled.setTextSize(2);
        oled.printf("%.1f", temp);
        oled.setTextSize(1);
        oled.printf("c");

        oled.setTextSize(2);
        oled.setTextCursor(68, 36);
        oled.printf("%.1f%%", humi);

        display();
    }

};

#endif