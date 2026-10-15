#include <encoder.h>
#include <context.h>
#include <nmt_states.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <stdlib.h>

static const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));


void print_data(unsigned long t_ms, double ref_speed, double actual_speed, double pwm_value) {
  printk("%lu,%.2f,%.2f,%.2f\n", t_ms, ref_speed, actual_speed, pwm_value);
}

static void read_line(char *buf, int len) {
  int i = 0;
  while (i < len - 1) {
    unsigned char c;
    if (uart_poll_in(uart, &c) != 0) continue;   // nothing arrived yet, keep polling
    if (c == '\n' || c == '\r') break;          // Enter ends the line
    buf[i++] = c;
  }
  buf[i] = '\0';                                 // terminate so atof knows where the text ends
}


int main() {
  Context context(&initialization);

  enc.init();
  status_led.init();

  printk("time_ms,ref_speed,actual_speed,pwm_value\n");
  context.start();


  while (1){
    unsigned char cmd;
    if (uart_poll_in(uart, &cmd) == 0) {

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
        char buf[16];
        read_line(buf, sizeof(buf));
        double kp = atof(buf);
        context.set_Kp(kp);
      }
      else if (cmd == 't') {
        char buf[16];
        read_line(buf, sizeof(buf));
        double ti = atof(buf);
        context.set_Ti(ti);
      }
      else if (cmd == 'l') {
        char buf[16];
        read_line(buf, sizeof(buf));
        char *law = buf;
        while (*law == ' ') law++; // skip leading spaces, like trim() did
        context.set_control_law(*law);
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
    unsigned int key = irq_lock();
    t = enc.ms_since_start;
    irq_unlock(key);

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