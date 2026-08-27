#include <avr/io.h>
#include <digital_out.h>

// Bound to Port B currently
Digital_out::Digital_out(uint8_t pin) 
: pinMask(1 << pin) {}

void Digital_out::init() {
    DDRB |= pinMask;
}

void Digital_out::set_hi() {
    PORTB |= pinMask;
}

void Digital_out::set_lo() {
    PORTB &= ~pinMask;
}

void Digital_out::toggle() {
    PORTB ^= pinMask;
}