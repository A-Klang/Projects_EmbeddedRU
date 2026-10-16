#include <motor_app.h>
#include <nmt_states.h>
#include <controllers.h>

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
