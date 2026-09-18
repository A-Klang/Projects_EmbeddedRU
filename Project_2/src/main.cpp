#include <util/delay.h>
#include <encoder.h>
#include <Arduino.h>
#include <timer_msec.h>
#include <p_controller.h>

void print_data(unsigned long t_ms, double ref_speed, double actual_speed, double pwm_value) {
  Serial.print(t_ms);
  Serial.print(",");
  Serial.print(ref_speed);
  Serial.print(",");
  Serial.print(actual_speed);
  Serial.print(",");
  Serial.println(pwm_value);
}

int main()
{

  Serial.begin(115200);
  enc.init();

  sei();
  P_cont.set_Kp(40);

  enc.ref_speed = 0;
  bool stepped = false;

  Serial.println("time_ms,ref_speed,actual_speed,pwm_value");

  while (1)
  {
    unsigned long t = enc.ms_since_start;
    if (!stepped && t >= 1000) {
      enc.ref_speed = 50; // Step
      stepped = true;
    }

    if (enc.counter == 10) {
      print_data(t, enc.ref_speed, enc.get_speed(), enc.pwm_value);
      enc.counter = 0;
    }
  }

  return 0;
}

