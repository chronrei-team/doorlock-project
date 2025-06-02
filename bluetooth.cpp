#ifndef _BLUETOOTH_
#define _BLUETOOTH_

#include "mbed.h"
#include <cstdio>

#define BT_TX_PIN PA_11
#define BT_RX_PIN PA_12

enum class BluetoothEvent {
    Open,
    Close,
    None
};

class DoorlockBluetooth {
private:
    Mutex lock;
    BufferedSerial pc;
    BufferedSerial bt;
    char inputBuffer[6];
public:
    DoorlockBluetooth() : pc(USBTX, USBRX), bt(BT_TX_PIN, BT_RX_PIN) { 
        pc.set_baud(9600);
        bt.set_baud(9600);
        inputBuffer[5] = '\0';
    }

    void readAlway() {
        while (true) {
            while (bt.readable()) {
                lock.lock();
                fill_n(inputBuffer, 6, '\0');
                bt.read(inputBuffer, 5);
                lock.unlock();
            }

            ThisThread::sleep_for(50ms);
        }
    }

    BluetoothEvent getEvent() {
        BluetoothEvent event = BluetoothEvent::None;

        lock.lock();
        printf("%s", inputBuffer);
        if (!strncmp(inputBuffer, "off", 3)) {
            event = BluetoothEvent::Close;
        }
        else if (!strncmp(inputBuffer, "on", 2)) {
            event = BluetoothEvent::Open;
        }
        fill_n(inputBuffer, 6, '\0');

        lock.unlock();
        return event;
    }
};

#endif