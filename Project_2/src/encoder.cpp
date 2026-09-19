#include <avr/io.h>
#include <avr/interrupt.h>
#include <encoder.h>
#include <p_controller.h>
#include <Arduino.h>
#include <timer_msec.h>
#include <analog_out.h>

Encoder enc(4, 3, 5, 0, 1); // C2=PD3, C1=PD4, LED=PB5, AIN1 = PB0, AIN2 = PB1
volatile uint8_t portd_history = 0xFF; // default is high because of pull-up

Timer_msec timer;

Encoder::Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t led_pin, uint8_t AIN1_port, uint8_t AIN2_port)
: c1(c1_pin), c2(c2_pin), led(led_pin), pwm_pin(AIN1_port), dir_pin(AIN2_port), last_c1(false), 
pos(0), speed_rpm(0) {}


void Encoder::init() {
    timer.init(period_ms);
    c1.init();
    c2.init();
    led.init();
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

float Encoder::get_speed() {
    return speed_rpm;
}

void Encoder::update() {
    led.set_hi();
    bool now_c1 = c1.is_hi();
    if (now_c1 == c2.is_hi())
        pos++;
    else
        pos--;
    last_c1 = now_c1;
    led.set_lo();
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
    enc.pwm_value = P_cont.update(enc.ref_speed, enc.get_speed());

    if (enc.pwm_value >= 0) 
    {
        enc.dir_pin.set_lo();
        enc.pwm_pin.set(enc.pwm_value);
    } 
    else 
    {
        enc.dir_pin.set_hi();
        enc.pwm_pin.set(255.0 - fabs(enc.pwm_value));
    }
}