#include <util/delay.h>
#include <encoder.h>
#include <Arduino.h>
#include <timer_msec.h>
#include <analog_out.h>

int main()
{

  Serial.begin(115200);
  enc.init();
  analog.init();
  
  sei();
  enc.ref_speed = 50;
  while (1)
  {
    if (enc.counter == 200){
      Serial.println(enc.get_speed());
      enc.counter = 0;
    }
  }

  return 0;
}

