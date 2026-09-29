#include <avr/io.h>
#include <digital_in.h>

// Bound to port D currently
Digital_in::Digital_in(uint8_t pin)
: pinMask(1 << pin) {}

void Digital_in::init() {
    DDRD &= ~pinMask; //input mode
    PORTD |= pinMask;
}

bool Digital_in::is_hi() {
    return (PIND & pinMask) != 0;
}
