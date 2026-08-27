#include <Arduino.h>
#include <util/delay.h>
#include <encoder.h>

Encoder enc(3, 4, 5); // C1=PD3, C2=PD4, LED = PB5)

void setup() {
  enc.init();
}

void loop() {
  enc.sample();
  _delay_us(140); // ~286/2
}