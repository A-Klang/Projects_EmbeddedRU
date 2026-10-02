#pragma once
#include <state.h>
#include <digital_out.h>
#include <stdint.h>

class Initialization : public State {
    public:
        void on_entry() override;
        void on_exit() override;
        void on_step() override;
};

class PreOperational : public State {
    public:
        void on_entry() override;
        void on_step() override;
        void on_reset() override;
        void on_fault() override;
        void on_set_operational() override;
        void on_set_Kp(double K_p) override;
        void on_set_Ti(double T_i) override;
        void on_set_control_law(char law) override;
    private:
        uint16_t blink_ticks = 0;
};

class Operational : public State {
    public:
        void on_entry() override;
        void on_exit() override;
        void on_step() override;
        void on_reset() override;
        void on_fault() override;
        void on_set_preoperational() override;
};

class Stopped : public State {
    public:
        void on_entry() override;
        void on_step() override;
        void on_reset() override;
        void on_set_operational() override;
        void on_set_preoperational() override;
    private:
        uint16_t blink_ticks = 0;
};

extern Initialization initialization;
extern Operational operational;
extern Digital_out status_led;
extern Stopped stopped;
extern PreOperational preoperational;
