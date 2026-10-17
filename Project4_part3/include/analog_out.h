#ifndef ANALOG_OUT_H
#define ANALOG_OUT_H

#include <stdint.h>
#include <digital_out.h>

class Analog_out
{
public:
    Analog_out(int pin);
    void init(int period_us = 120);
    void set(double duty_cycle);

private:
    uint32_t period_ns;
public:    
    Digital_out pin;
};

#endif // ANALOG_OUT_H