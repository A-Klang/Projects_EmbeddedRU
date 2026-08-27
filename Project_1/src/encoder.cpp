#include <avr/io.h>
#include <encoder.h>

Encoder::Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t led_pin)
: c1(c1_pin), c2(c2_pin), led(led_pin), last_c1(false), pos(0) {}


void Encoder::init() {
    c1.init();
}

int Digital_in::position() {

}