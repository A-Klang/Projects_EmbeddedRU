#include <avr/io.h>
#include <avr/interrupt.h>
#include <p_controller.h>
#include <Arduino.h>
#include <timer_msec.h>

P_controller P_cont(1);

P_controller::P_controller(double Kp) {
    K_p = Kp;
}

void P_controller::update(double ref, double actual) {
    pwm_value = K_p * (ref - actual);
}