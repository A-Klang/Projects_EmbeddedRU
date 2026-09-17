#pragma once

class Timer_usec
{
public:
    Timer_usec();
    void init(uint16_t period_us, int duty_cycle);
    void set_duty_cycle(int duty_cycle);
};