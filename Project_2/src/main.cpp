#include <util/delay.h>
#include <encoder.h>
#include <Arduino.h>
#include <timer_msec.h>

int main()
{

  Serial.begin(115200);
  enc.init();
  
  sei();
  while (1)
  {

  }

  return 0;
}

