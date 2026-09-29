#include <Arduino.h>
#include <encoder.h>
#include <context.h>
#include <nmt_states.h>

void print_data(unsigned long t_ms, double ref_speed, double actual_speed, double pwm_value) {
  Serial.print(t_ms);
  Serial.print(",");
  Serial.print(ref_speed);
  Serial.print(",");
  Serial.print(actual_speed);
  Serial.print(",");
  Serial.println(pwm_value);
}

int main() {
  Context context(&initialization);

  Serial.begin(115200);
  enc.init();
  status_led.init();
  sei();

  Serial.println("time_ms,ref_speed,actual_speed,pwm_value");
  context.start();


  while (1){
    if (Serial.available() > 0) {
      char cmd = Serial.read();
      if (cmd == 'r') {
        context.reset();
      }
    }

    if (enc.control_tick) {
      enc.control_tick = false;
      context.step();
    }

    unsigned long t = enc.ms_since_start;
    if (t >= 8000) {
      enc.ref_speed = 60;
    }
    else if(t >= 1000) {
      enc.ref_speed = 40;
    }

    if (enc.counter >= 10) {
      enc.counter = 0;
      print_data(t, enc.ref_speed, enc.get_speed(), enc.pwm_value);
    }
  }

  return 0;
}