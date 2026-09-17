#include <util/delay.h>
#include <encoder.h>
#include <Arduino.h>
#include <timer_msec.h>
#include <p_controller.h>

int main()
{

  Serial.begin(115200);
  enc.init();
  
  sei();
  P_cont.set_Kp(1);

  enc.ref_speed = 0; // Start at rest
  _delay_ms(2000);
  enc.ref_speed = 50; // Step

  int last_print_pos = 0;
  while (1)
  {
    if (enc.counter == 20){
      int pos_now = enc.get_position();
      int delta = pos_now - last_print_pos;
      float display_rpm = ((float)delta / 20.0f) / 2100.0f * 60000.0f;
      Serial.print(display_rpm);
      Serial.print(",");
      Serial.println(enc.last_pwm);
      last_print_pos = pos_now;
      enc.counter = 0;
    }
  }

  return 0;
}

