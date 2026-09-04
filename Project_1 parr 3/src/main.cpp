#include <util/delay.h>
#include <encoder.h>
#include <Arduino.h>


int main()
{
  Serial.begin(115200);
  enc.init();
  
  sei();
  while (1)
  {
    Serial.println(enc.position());
  }

  return 0;
}

