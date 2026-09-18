#include <avr/io.h>
#include <avr/interrupt.h>
#include <analog_out.h>
#include <encoder.h>


Analog_out::Analog_out(int pin) : pin(pin) {}

void Analog_out :: init(int period_us) {
    pin.init();
    timer.init(period_us, 0);
}
void Analog_out :: set(double duty_cycle){
    if (duty_cycle < 0) duty_cycle = 0;
    if (duty_cycle > 255) duty_cycle = 255;
    timer.set_duty_cycle(duty_cycle);
}

// COMPA fires at TOP (start of each period), COMPB at OCR0B, so the pin is
// HIGH for OCR0B ticks: high-time is proportional to the commanded value.
ISR(TIMER0_COMPA_vect)
{
    enc.pwm_pin.pin.set_hi();
}
ISR(TIMER0_COMPB_vect)
{
    enc.pwm_pin.pin.set_lo();
}