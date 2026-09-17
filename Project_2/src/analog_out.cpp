#include <avr/io.h>
#include <avr/interrupt.h>
#include <analog_out.h>
#include <encoder.h>

Analog_out analog(3); //pin 3 = PB3

Analog_out::Analog_out(int pin) : pin(pin) {}

void Analog_out :: init(int period_us) {
    pin.init();
    timer.init(period_us, 0);
}
void Analog_out :: set(int duty_cycle){
    if (duty_cycle < 0) duty_cycle = 0;
    if (duty_cycle > 100) duty_cycle = 100;
    timer.set_duty_cycle(duty_cycle);
}

ISR(TIMER0_COMPA_vect)
{
    enc.AIN1.pin.set_hi();
}
ISR(TIMER0_COMPB_vect)
{
    enc.AIN1.pin.set_lo();
}