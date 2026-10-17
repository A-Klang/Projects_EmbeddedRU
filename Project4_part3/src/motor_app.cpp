#include <motor_app.h>
#include <nmt_states.h>
#include <controllers.h>
#include <encoder.h>

// Every function locks app_mutex, since the CAN thread and the work queue reach the application through here

NmtState MotorApp::nmt_state() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    State *s = context_.current();
    NmtState state = NmtState::Initialization;
    if (s == &operational) state = NmtState::Operational;
    else if (s == &preoperational) state = NmtState::PreOperational;
    else if (s == &stopped) state = NmtState::Stopped;
    k_mutex_unlock(&app_mutex); // one return after the unlock, so the mutex is always released
    return state;
}

void MotorApp::nmt_start() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    context_.set_operational();
    k_mutex_unlock(&app_mutex);
}

void MotorApp::nmt_stop() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    context_.fault(); // Stopped is entered through the same event as a fault
    k_mutex_unlock(&app_mutex);
}

void MotorApp::nmt_preoperational() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    context_.set_preoperational();
    k_mutex_unlock(&app_mutex);
}

void MotorApp::nmt_reset_node() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    // Reset node also resets the application parameters, same defaults as in controllers.cpp
    p.Kp = 40;
    pi.Kp = 40;
    pi.Ti = 0.1;
    operational.set_controller(&pi);
    context_.reset();
    k_mutex_unlock(&app_mutex);
}

void MotorApp::nmt_reset_communication() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    context_.reset();
    k_mutex_unlock(&app_mutex);
}

float MotorApp::velocity_actual() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    float speed = enc.get_speed();
    k_mutex_unlock(&app_mutex);
    return speed;
}

float MotorApp::duty_cycle() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    float duty_cycle = enc.pwm_value / 255.0f;
    k_mutex_unlock(&app_mutex);
    return duty_cycle;
}

uint8_t MotorApp::controller_type() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    uint8_t type = operational.get_controller() == &pi ? 1 : 0;
    k_mutex_unlock(&app_mutex);
    return type;
}

void MotorApp::set_controller_type(uint8_t type) {
    k_mutex_lock(&app_mutex, K_FOREVER);
    context_.set_control_law(type ? 'i' : 'p');
    k_mutex_unlock(&app_mutex);
}

float MotorApp::kp() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    float kp = operational.get_controller()->Kp;
    k_mutex_unlock(&app_mutex);
    return kp;
}

void MotorApp::set_kp(float kp) {
    k_mutex_lock(&app_mutex, K_FOREVER);
    context_.set_Kp(kp);
    k_mutex_unlock(&app_mutex);
}

float MotorApp::ti() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    float ti = pi.Ti;
    k_mutex_unlock(&app_mutex);
    return ti;
}

void MotorApp::set_ti(float ti) {
    k_mutex_lock(&app_mutex, K_FOREVER);
    context_.set_Ti(ti);
    k_mutex_unlock(&app_mutex);
}

int32_t MotorApp::target_velocity() {
    k_mutex_lock(&app_mutex, K_FOREVER);
    int32_t rpm = enc.ref_speed;
    k_mutex_unlock(&app_mutex);
    return rpm;
}

void MotorApp::set_target_velocity(int32_t rpm) {
    k_mutex_lock(&app_mutex, K_FOREVER);
    enc.ref_speed = rpm;
    k_mutex_unlock(&app_mutex);
}
