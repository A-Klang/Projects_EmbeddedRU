#include <nmt_states.h>
#include <context.h>
#include <encoder.h>
#include <controller.h>
#include <controllers.h>
#include <zephyr/kernel.h>
#include <math.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nmt_states, LOG_LEVEL_INF);

Initialization initialization;
Operational operational(&pi); // controller is passed to the motor controller, switched in PreOperational
Digital_out status_led(5); // led0 (PB3) on the Nucleo
Stopped stopped;
PreOperational preoperational;
void (*on_boot_up)() = nullptr;


void Initialization::on_entry() {
    enc.stop_motor();
    enc.reset_position();

    enc.ref_speed = 0;
    enc.ms_since_start = 0;
}

void Initialization::on_exit() {
    LOG_INF("Boot-up");
    if (on_boot_up) on_boot_up();
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
}

void PreOperational::on_set_Kp(double K_p) {
    // Kp goes to the selected controller, so P and PI can keep their own tuning
    operational.get_controller()->Kp = K_p;
    LOG_INF("%s Kp set to %.2f", operational.get_controller() == &pi ? "PI" : "P", K_p);
}

void PreOperational::on_set_Ti(double T_i) {
    // Ti is only used by the PI controller, so it is set there whichever law is selected
    if (T_i > 0) {
        pi.Ti = T_i;
        LOG_INF("PI Ti set to %.2f", T_i);
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
        operational.set_controller(&pi);
        LOG_INF("Control law: PI");
    }
    else {
        operational.set_controller(&p);
        LOG_INF("Control law: P");
    }
}

void Operational::on_entry() {
    status_led.set_hi();
    controller_->reset();
    setpoint.reset();
    unsigned int key = irq_lock();
    entry_ms = enc.ms_since_start;
    irq_unlock(key);
}

void Operational::on_step() {
    double ref = setpoint.update(enc.ref_speed); // ramps the reference towards the target
    enc.pwm_value = controller_->update(ref, enc.get_speed());


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