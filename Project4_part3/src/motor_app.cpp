#include <motor_app.h>
#include <nmt_states.h>
#include <controllers.h>
#include <encoder.h>

NmtState MotorApp::nmt_state() {
    State *s = context_.current();
    if (s == &operational) return NmtState::Operational;
    if (s == &preoperational) return NmtState::PreOperational;
    if (s == &stopped) return NmtState::Stopped;
    return NmtState::Initialization;
}

void MotorApp::nmt_start() { context_.set_operational(); }

void MotorApp::nmt_stop() { context_.fault(); } // Stopped is entered through the same event as a fault

void MotorApp::nmt_preoperational() { context_.set_preoperational(); }

void MotorApp::nmt_reset_node() {
    // Reset node also resets the application parameters, same defaults as in controllers.cpp
    p.Kp = 40;
    pi.Kp = 40;
    pi.Ti = 0.1;
    operational.set_controller(&pi);
    context_.reset();
}

void MotorApp::nmt_reset_communication() { context_.reset(); }

float MotorApp::velocity_actual() { return enc.get_speed(); }
float MotorApp::duty_cycle() { return enc.pwm_value / 255.0f; }

uint8_t MotorApp::controller_type() { return operational.get_controller() == &pi ? 1 : 0; }
void MotorApp::set_controller_type(uint8_t type) { context_.set_control_law(type ? 'i' : 'p'); }

float MotorApp::kp() { return operational.get_controller()->Kp; }
void MotorApp::set_kp(float kp) { context_.set_Kp(kp); }

float MotorApp::ti() { return pi.Ti; }
void MotorApp::set_ti(float ti) { context_.set_Ti(ti); }

int32_t MotorApp::target_velocity() { return enc.ref_speed; }
void MotorApp::set_target_velocity(int32_t rpm) { enc.ref_speed = rpm; }
