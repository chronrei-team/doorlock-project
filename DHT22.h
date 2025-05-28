#ifndef MBED_DHT22_H
#define MBED_DHT22_H

#include "mbed.h"

class DHT22 {
private:
    int _temperature,_humidity;
    PinName _data_pin;
    EventQueue *event;
    int taskId = -1;
public:
    DHT22(PinName, EventQueue *_event);
    bool sample();
    int getTemperature();
    int getHumidity();
    void sampleAlway();
    void sampleStop();
};

#endif