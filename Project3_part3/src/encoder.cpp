#include <avr/io.h>
#include <avr/interrupt.h>
#include <encoder.h>
#include <Arduino.h>
#include <timer_msec.h>
#include <analog_out.h>
#include <util/atomic.h>


Encoder enc(4, 3, 0, 1, 2); // C2=PD3, C1=PD4, AIN1 = PB0, AIN2 = PB1, FLT = PD2
volatile uint8_t portd_history = 0xFF; // default is high because of pull-up

Timer_msec timer;

Encoder::Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t AIN1_port, uint8_t AIN2_port, uint8_t flt_pin)
: pwm_pin(AIN1_port), dir_pin(AIN2_port), c1(c1_pin), c2(c2_pin), flt(flt_pin), last_c1(false),
pos(0), speed_rpm(0) {}


void Encoder::init() {
    timer.init(period_ms);
    c1.init();
    c2.init();
    flt.init();
    pwm_pin.init(); //Previously AIN1
    dir_pin.init(); //Previously AIN2
    last_c1 = c1.is_hi();
    pos = 0;
    counter = 0;

    portd_history = PIND;
    PCMSK2 |= (1 << PCINT20); // Enable pin-change interrupt for PD4
    PCICR |= (1 << PCIE2);    // Enable pin-change interrupt group for PORTD
}

int Encoder::get_position() {
    return pos;
}

bool Encoder::has_fault() {
    return flt.is_lo();
}

void Encoder::brake() {
    dir_pin.set_hi();
    pwm_pin.set(255); //Set both pins high to actively brake
    pwm_value = 0;
}

float Encoder::get_speed() {
    float s;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {s = speed_rpm;}
    return s;
}

void Encoder::stop_motor() {
    dir_pin.set_lo();
    pwm_pin.set(0); // Set both pins low to "coast"
    pwm_value = 0;
}

void Encoder::reset_position() {
    pos = 0;
    for (uint8_t i = 0; i < window_ticks; i++) {
        window_pos[i] = 0;
    }
}

void Encoder::update() {
    bool now_c1 = c1.is_hi();
    if (now_c1 == c2.is_hi())
        pos++;
    else
        pos--;
    last_c1 = now_c1;
}

void Encoder::sample_speed() {
    int now_pos = pos;
    int delta = now_pos - window_pos[tick]; // window_pos[tick] is the position `window_ticks`(10ms) ticks ago
    window_pos[tick] = now_pos;
    tick = (tick + 1) % window_ticks;
    speed_rpm = (((float)delta / (period_ms * window_ticks)) / counts_per_rev) * 60000.0f;
}


ISR (PCINT2_vect)
{
    uint8_t changed_bits = PIND ^ portd_history;
    portd_history = PIND;
    if(changed_bits & (1 << PIND4)) // PD4 changed
    enc.update();
}

ISR(TIMER1_COMPA_vect) 
{
    enc.counter++;
    enc.ms_since_start++;
    enc.sample_speed();
    enc.control_tick = true;
}