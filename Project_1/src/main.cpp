#include <util/delay.h>
#include <encoder.h>

int main()
{
  Encoder enc(3, 4, 5); // C1=PD3, C2=PD4, LED=PB5

  enc.init();

  while (1)
  {
    enc.sample();
    _delay_us(140); // ~286/2
  }

  return 0;
}