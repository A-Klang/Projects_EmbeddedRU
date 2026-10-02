#include <nmt_states.h>
#include <context.h>
#include <encoder.h>
#include <controller.h>
#include <controllers.h>
#include <Arduino.h>

Initialization initialization;
Operational operational;
Digital_out status_led(5); // PB5 = nano led
Stopped stopped;
PreOperational preoperational;

void Initialization::on_entry() {
    enc.stop_motor();
    enc.reset_position();

    enc.ref_speed = 0;
    enc.ms_since_start = 0;
}

void Initialization::on_exit() {
    Serial.println("Boot-up");
}

void Initialization::on_step() {
    context_->transition_to(&preoperational);
}

void PreOperational::on_entry() {
    blink_ticks = 0;
}

void PreOperational::on_step() {
    if (++blink_ticks >= 500) {
        blink_ticks = 0;
        status_led.toggle();
    }
    // if (Serial.available() > 0) {
    //   String cmd = Serial.readString();
    //   if (strcmp(cmd.c_str(), "kp")) {
    //     set_Kp(Serial.parseInt());
    //   }

    //   else if (strcmp(cmd.c_str(), "ti")) {
    //     set_Ti(Serial.parseInt());
    //   }
    // }
}

void PreOperational::on_set_Kp(double K_p) {
    controller->Kp = K_p;
}

void PreOperational::on_set_Ti(double T_i) {
    if (T_i > 0) {
        controller->Ti = T_i;
    }
}

void PreOperational::on_reset() {
    context_->transition_to(&initialization);
}

void PreOperational::on_set_operational() {
    context_->transition_to(&operational);
}

void PreOperational::on_fault() {
    context_->transition_to(&stopped);
}

void PreOperational::on_set_control_law(char law) {
    if (law == 'i') {
        controller = &pi;
    }
    else {
        controller = &p;
    }
}

void Operational::on_entry() {
    status_led.set_hi();
    controller->reset();
}

void Operational::on_step() {
    //enc.pwm_value = P_cont.update(enc.ref_speed, enc.get_speed());
    enc.pwm_value = controller->update(enc.ref_speed, enc.get_speed());

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

void Operational::on_fault() {
    context_->transition_to(&stopped);
}

void Operational::on_set_preoperational() {
    context_->transition_to(&preoperational);
}

void Stopped::on_entry() {
    enc.brake();
    blink_ticks = 0;
}

void Stopped::on_step() {
    if (++blink_ticks >= 250) { //Toggle every 250ms = 2hz blink
        blink_ticks = 0;
        status_led.toggle();
    }
}

void Stopped::on_reset() {
    context_->transition_to(&initialization);
}

void Stopped::on_set_operational() {
    context_->transition_to(&operational);
}

void Stopped::on_set_preoperational() {
    context_->transition_to(&preoperational);
}