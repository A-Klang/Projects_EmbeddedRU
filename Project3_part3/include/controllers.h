#pragma once
#include <controller.h>

class P_controller : public Controller {
    public:
        double update(double ref, double actual) override;
}

class Initialization : public State {
    public:
        void on_entry() override;
        void on_exit() override;
        void on_step() override;
};

class Operational : public State {
    public:
        void on_entry() override;
        void on_exit() override;
        void on_step() override;
        void on_reset() override;
        void on_fault() override;
};

class Stopped : public State {
    public:
        void on_entry() override;
        void on_step() override;
        void on_reset() override;
        void on_set_operational() override;
    private:
        uint16_t blink_ticks = 0;
};

extern Initialization initialization;
extern Operational operational;
extern Digital_out status_led;
extern Stopped stopped;
