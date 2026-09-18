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
pos(0), last_sample_pos(0), speed_rpm(0) {}


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
    if (counter2 != 10) return; // wait for the full 10-tick window (must match ISR's counter2 threshold) for finer resolution
    int now_pos = pos;
    int delta = now_pos - last_sample_pos;
    speed_rpm = (((float)delta / (period_ms * 10.0f)) / counts_per_rev) * 60000.0f;
    last_sample_pos = now_pos;
    counter2 = 0;
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
    enc.counter2++;
    enc.ms_since_start++;
    if (enc.counter2 == 10) {
        enc.sample_speed();
        if (!enc.open_loop) {
            enc.pwm_value = P_cont.update(enc.ref_speed, enc.get_speed());
        }
    }

    if (!enc.open_loop) {
        // Forward: dir_pin low, so the bridge sees (0,0)=coast / (1,0)=drive and
        // drive time tracks the commanded value directly.
        // Reverse: dir_pin high gives (0,1)=drive / (1,1)=brake, so drive time is
        // the LOW time - the command has to be inverted to stay monotonic.
        if (enc.pwm_value >= 0) {
            enc.dir_pin.set_lo();
            enc.pwm_pin.set(enc.pwm_value);
        } else {
            enc.dir_pin.set_hi();
            enc.pwm_pin.set(255.0 - fabs(enc.pwm_value));
        }
    }
} 