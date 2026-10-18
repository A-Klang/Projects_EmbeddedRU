#pragma once
#include <stdint.h>

enum class NmtState { Initialization, PreOperational, Operational, Stopped };

class NodeApp {
public:
    virtual NmtState nmt_state() = 0;
    virtual void nmt_start() = 0;
    virtual void nmt_stop() = 0;
    virtual void nmt_preoperational() = 0;
    virtual void nmt_reset_node() = 0;
    virtual void nmt_reset_communication() = 0;
    virtual float velocity_actual() = 0;   // RPM
    virtual float duty_cycle() = 0;        // -1 to 1
    virtual uint8_t controller_type() = 0; // 0 = P, 1 = PI
    virtual void set_controller_type(uint8_t type) = 0;
    virtual float kp() = 0;
    virtual void set_kp(float kp) = 0;
    virtual float ti() = 0;
    virtual void set_ti(float ti) = 0;
    virtual int32_t target_velocity() = 0;
    virtual void set_target_velocity(int32_t rpm) = 0;


    virtual uint32_t profile_acc() = 0;  // RPM/s, 0 = no limit
    virtual void set_profile_acc(uint32_t rpm_s) = 0;
    virtual uint32_t profile_dec() = 0;
    virtual void set_profile_dec(uint32_t rpm_s) = 0;
    
    virtual ~NodeApp() {}
};