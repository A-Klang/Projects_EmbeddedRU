#include <nmt_states.h>
#include <context.h>
#include <encoder.h>
#include <p_controller.h>
#include <Arduino.h>

Initialization initialization;
Operational operational;
Digital_out status_led(5); // PB5 = nano led

void Initialization::on_entry() {
    enc.stop_motor();
    enc.reset_position();
    P_cont.set_Kp(40);
    enc.ref_speed = 0;
    enc.ms_since_start = 0;
}

void Initialization::on_exit() {
    Serial.println("Boot-up");
}

void Initialization::on_step() {
    context_->transition_to(&operational);
}

void Operational::on_entry() {
    status_led.set_hi();
}

void Operational::on_step() {
    enc.pwm_value = P_cont.update(enc.ref_speed, enc.get_speed());

    if (enc.pwm_value >= 0) {
        enc.dir_pin.set_lo();
        enc.pwm_pin.set(enc.pwm_value);
    } else {
        enc.dir_pin.set_hi();
        enc.pwm_pin.set(255.0 - fabs(enc.pwm_value));
    }
}

void Operational::on_exit() {
    enc.stop_motor();
}

void Operational::on_reset() {
    context_->transition_to(&initialization);
}
