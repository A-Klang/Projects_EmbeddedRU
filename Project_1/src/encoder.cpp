#include <avr/io.h>
#include <encoder.h>

Encoder::Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t led_pin)
: c1(c1_pin), c2(c2_pin), led(led_pin), last_c1(false), pos(0) {}


void Encoder::init() {
    c1.init();
    c2.init();
    led.init();
    last_c1 = c1.is_hi();
    pos = 0;
}

int Encoder::position() {
    return pos;
}

void Encoder::sample() {
    bool now_c1 = c1.is_hi();
    if (now_c1 != last_c1) {
        if (now_c1 == c2.is_hi())
            pos++;
        else
            pos--;
        last_c1 = now_c1;
        led.set_hi();
    } 
    else {
        led.set_lo();
    }
}