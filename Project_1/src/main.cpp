#include <util/delay.h>
#include <encoder.h>
#include <Arduino.h>

int main()
{
  Serial.begin(9600);
  sei(); // Enable UART interrupts used by Serial.
  Encoder enc(4, 3, 5); // C2=PD3, C1=PD4, LED=PB5

  enc.init();

  while (1)
  {
    enc.sample();

    Serial.println(enc.position());

    _delay_us(140); // ~286/2
  }

  return 0;
}
