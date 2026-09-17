#include <avr/io.h>
#include <avr/interrupt.h>
#include <p_controller.h>
#include <Arduino.h>
#include <timer_msec.h>

P_controller P_cont(1);

P_controller::P_controller(double Kp) {
    K_p = Kp;
}

double P_controller::update(double ref, double actual) {
    return K_p * (ref - actual);
}

void P_controller::set_Kp(double Kp) {
    K_p = Kp;
}