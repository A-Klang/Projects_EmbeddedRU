#include <avr/io.h>
#include <digital_in.h>

Digital_in::Digital_in(uint8_t pin) 
: pinMask(1 << pin) {}

void Digital_in::init() {
    DDRB &= ~pinMask; //input mode
    PORTB |= pinMask; 
}

int Digital_in::position() {

}