#include <avr/io.h>
#include <avr/interrupt.h>
#include <encoder.h>
#include <Arduino.h>
#include <timer_msec.h>

Encoder enc(4, 3, 5, 0, 1); // C2=PD3, C1=PD4, LED=PB5, AIN1 = PB0, AIN2 = PB1
volatile uint8_t portd_history = 0xFF; // default is high because of pull-up

Timer_msec timer;

Encoder::Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t led_pin, uint8_t AIN1_port, uint8_t AIN2_port)
: c1(c1_pin), c2(c2_pin), led(led_pin), AIN1(AIN1_port), AIN2(AIN2_port), last_c1(false), 
pos(0), last_sample_pos(0), speed_rpm(0) {}


void Encoder::init() {
    timer.init(period_ms);
    c1.init();
    c2.init();
    led.init();
    AIN1.init();
    AIN2.init();
    AIN1.set_lo();
    AIN2.set_hi();
    last_c1 = c1.is_hi();
    pos = 0;

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
    int delta = now_pos - last_sample_pos;
    speed_rpm = ((delta / period_ms) / counts_per_rev) * 60000.0f; // pulses per ms divided by pulses per rev = revs per ms, scale to 1min.
    last_sample_pos = now_pos;
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
    enc.sample_speed();
    float v = enc.get_speed();
    if  ((v > 0) && (v < 50.1)) {
        enc.counter++;
    }
    if (v > 50.1)
    {
        Serial.println(enc.counter);
    }
} 