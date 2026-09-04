#include <util/delay.h>
#include <encoder.h>
#include <Arduino.h>

volatile uint8_t portd_history = 0xFF; // default is high because of pull-up
Encoder enc(4, 3, 5); // C2=PD3, C1=PD4, LED=PB5

int main()
{
  Serial.begin(115200);
  enc.init();

  portd_history = PIND;
  PCMSK2 |= (1 << PCINT20); // Enable pin-change interrupt for PD4
  PCICR |= (1 << PCIE2);    // Enable pin-change interrupt group for PORTD

  sei();

  while (1)
  {

    Serial.println(enc.position());
  }

  return 0;
}

ISR (PCINT2_vect)
{
  uint8_t changed_bits;
  changed_bits = PIND ^ portd_history;
  portd_history = PIND;
  if(changed_bits & (1 << PIND4)) {// PD4 changed
    enc.update();
  } 
  enc.led.set_lo();
}
