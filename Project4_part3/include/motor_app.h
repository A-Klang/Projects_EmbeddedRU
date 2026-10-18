#pragma once
#include <node_app.h>
#include <context.h>
#include <zephyr/kernel.h>

class MotorApp : public NodeApp {
public:
    MotorApp(Context &context) : context_(context) {}
    NmtState nmt_state() override;
    void nmt_start() override;
    void nmt_stop() override;
    void nmt_preoperational() override;
    void nmt_reset_node() override;
    void nmt_reset_communication() override;
    float velocity_actual() override;
    float duty_cycle() override;
    uint8_t controller_type() override;
    void set_controller_type(uint8_t type) override;
    float kp() override;
    void set_kp(float kp) override;
    float ti() override;
    void set_ti(float ti) override;
    int32_t target_velocity() override;
    void set_target_velocity(int32_t rpm) override;
    uint32_t profile_acc() override;
    void set_profile_acc(uint32_t rpm_s) override;
    uint32_t profile_dec() override;
    void set_profile_dec(uint32_t rpm_s) override;



private:
    Context &context_;
};

extern struct k_mutex app_mutex;

