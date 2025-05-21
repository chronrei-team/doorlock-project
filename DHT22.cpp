#include "DHT22.h"

#define timeout 200

DHT22::DHT22(PinName pin) {
    _data_pin = pin;
    DigitalInOut dht(_data_pin);
    dht.output();
    dht.write(1);
}

int DHT22::getTemperature() {
    return _temperature;
}

int DHT22::getHumidity() {
    return _humidity;
}

bool DHT22::sample() {
    
    Timer t;
    DigitalInOut dht(_data_pin);
    int dht22_dat [5];

    __disable_irq();    // Disable Interrupts
    t.start();

    // send start signal
    dht.output();
    dht.write(0);
    wait_us(1000); // 1ms delay
    dht.write(1);

    // receive response signal
    dht.input();
    while (dht.read());
    wait_us(40);
    wait_us(80);

    // receive 40bit data
    int result=0;
    for (int i=0; i<5; i++) {
        result=0; 
        for (int j=0; j<8; j++) {
            while(dht.read());
            while(!dht.read());
            t.reset();
            while(dht.read());
            int p;
            if(t.read_us()>50)
                p = 1;
            else
                p = 0;
            p=p <<(7-j);
            result=result|p;
        }
        dht22_dat[i] = result;
    }
    // send sleep signal
    dht.output();
    dht.write(1);

    t.stop();
    __enable_irq();

    int dht22_check_sum;
    dht22_check_sum=dht22_dat[0]+dht22_dat[1]+dht22_dat[2]+dht22_dat[3];
    dht22_check_sum= dht22_check_sum%256;
    if (dht22_check_sum==dht22_dat[4]) {
        _humidity=dht22_dat[0]*256+dht22_dat[1];
        _temperature=dht22_dat[2]*256+dht22_dat[3];
        
        return true;
    }
    __enable_irq();
    return false;
}
