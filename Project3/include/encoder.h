#pragma once

#include <digital_in.h>
#include <digital_out.h>
#include <analog_out.h>

class Encoder
{
    public:
        Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t led_pin, uint8_t AIN1_port, uint8_t AIN2_port);
        void init();
        void update(); // called from ISR(PCINT2_vect), not meant to be called directly
        void sample_speed();
        int get_position();
        float get_speed();
        void set_speed();
        volatile int counter = 0;
        volatile unsigned long ms_since_start = 0; // increments once per Timer1 tick (period_ms)
        volatile double ref_speed;
        volatile double last_pwm = 0; // most recent PWM value returned by P_cont.update(), for reporting
        volatile double pwm_value = 0;
        Analog_out pwm_pin; // was AIN1, drives H-bridge
        Digital_out dir_pin; // was AIN2, fixed direction level, not PWM


    private:
        Digital_in c1;
        Digital_in c2;
        Digital_out led;
        bool last_c1;
        static constexpr float period_ms = 1;
        static constexpr float counts_per_rev = 2100.0;
        static constexpr uint8_t window_ticks = 10;

        volatile int pos;
        volatile int window_pos[window_ticks] = {};
        volatile uint8_t tick = 0;
        volatile float speed_rpm;
};

extern Encoder enc;