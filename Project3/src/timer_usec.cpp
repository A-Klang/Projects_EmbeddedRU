#include <avr/io.h>
#include <avr/interrupt.h>
#include <timer_usec.h>
    
Timer_usec::Timer_usec(){}

void Timer_usec:: init(uint16_t period_us, double duty_cycle){
    uint32_t ticks = ((uint32_t)period_us * 2) -1;

    TCCR0A = 0; // Setting timer1 to normal operation
    TCCR0B = 0;
    TCNT0 = 0; // Initialize counter value
    OCR0A = (uint8_t) ticks;
    OCR0B = (uint8_t)((double)OCR0A * duty_cycle / 255.0);


    TCCR0A |= (1<<WGM01);
    TIMSK0 |= (1<<OCIE0A);
    TIMSK0 |= (1<<OCIE0B);
    TCCR0B |= (1<<CS01);


}

void Timer_usec :: set_duty_cycle(double duty_cycle) {
    double ticks = (double)OCR0A * duty_cycle / 255.0;
    // OCR0B == OCR0A makes both compare matches fire on the same tick; COMPA
    // sets the pin high and COMPB clears it microseconds later, collapsing the
    // pulse to ~0% instead of 100%.
    if (ticks > OCR0A - 1) ticks = OCR0A - 1;
    OCR0B = (uint8_t)ticks;
}