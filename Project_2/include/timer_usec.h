#pragma once

class Timer_usec
{
public:
    Timer_usec();
    void init(uint16_t period_us, double duty_cycle);
    void set_duty_cycle(double duty_cycle);
};