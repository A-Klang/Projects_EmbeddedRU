#ifndef ANALOG_OUT_H
#define ANALOG_OUT_H

#include <timer_usec.h>
#include <digital_out.h>

class Analog_out
{
public:
    Analog_out(int pin);
    void init(int period_us = 120);
    void set(int duty_cycle);

private:
    Timer_usec timer;
public:    
    Digital_out pin;
};

extern Analog_out analog;

#endif // ANALOG_OUT_H