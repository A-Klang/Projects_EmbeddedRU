#pragma once

class Timer_msec
{
public:
    Timer_msec();
    void init(int period_ms = 200, int duty_cycle = 50);
    void set_duty_cycle(int duty_cycle);
};