#include <Arduino.h>
#include <encoder.h>
#include <context.h>
#include <nmt_states.h>
#include <util/atomic.h>
#include <string.h>

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
      else if (cmd == 'o') {
        context.set_operational();
      }
      else if (cmd == 'p') {
        context.set_preoperational();
      }
      else if (cmd == 'k') {
        String input = Serial.readStringUntil('\n');
        input.trim();
        double kp = input.toDouble();
        context.set_Kp(kp);
      }
      else if (cmd == 't') {
        String input = Serial.readStringUntil('\n');
        input.trim();
        double ti = input.toDouble();
        context.set_Ti(ti);
        } 
      else if (cmd == 'l') {
        String input = Serial.readStringUntil('\n');
        input.trim();
        context.set_control_law(*input.c_str());
      }
      }

    if (enc.has_fault()) {
      context.fault();
    }

    if (enc.control_tick) {
      enc.control_tick = false;
      context.step();
    }
    unsigned long t;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { t = enc.ms_since_start; }

    // Load test profile, timed from entering Operational: 0 RPM until 1s, then constant 40 RPM.
    // The step test used 0 RPM until 1s, 40 RPM until 8s, then 60 RPM.
    unsigned long t_op = t - operational.entry_ms;
    if (t_op >= 1000) {
      enc.ref_speed = 40;
    }
    else {
      enc.ref_speed = 0;
    }

    if (enc.counter >= 10) {
      enc.counter = 0;
      print_data(t, enc.ref_speed, enc.get_speed(), enc.pwm_value);
    }
  }

  return 0;
}