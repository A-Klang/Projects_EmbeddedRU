#include <avr/io.h>
#include <avr/interrupt.h>
#include <encoder.h>
#include <Arduino.h>
#include <timer_msec.h>

Encoder enc(4, 3, 5); // C2=PD3, C1=PD4, LED=PB5
volatile uint8_t portd_history = 0xFF; // default is high because of pull-up
int last_pos = 0;
int delta_pos = 0;
float period_ms = 200;
float v = 0;

Timer_msec timer;

Encoder::Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t led_pin)
: c1(c1_pin), c2(c2_pin), led(led_pin), last_c1(false), pos(0) {}


void Encoder::init() {
    timer.init(period_ms);
    c1.init();
    c2.init();
    led.init();
    last_c1 = c1.is_hi();
    pos = 0;

    portd_history = PIND;
    PCMSK2 |= (1 << PCINT20); // Enable pin-change interrupt for PD4
    PCICR |= (1 << PCIE2);    // Enable pin-change interrupt group for PORTD
}

int Encoder::position() {
    return pos;
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

ISR (PCINT2_vect)
{
  uint8_t changed_bits = PIND ^ portd_history;
  portd_history = PIND;
  if(changed_bits & (1 << PIND4)) // PD4 changed
    enc.update();
}

ISR(TIMER1_COMPA_vect) 
  {
    int now_pos = enc.position();
    delta_pos = (now_pos-last_pos);
    v = ((delta_pos/200.0)/2100)*60000; // pulses per ms divided by pulses per rev = revs per ms, scale to 1min.
    last_pos = now_pos;
    Serial.print("v = ");
    Serial.println(v);
  } 