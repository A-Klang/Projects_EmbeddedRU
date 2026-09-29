#pragma once
#include <state.h>
#include <digital_out.h>

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
};

extern Initialization initialization;
extern Operational operational;
extern Digital_out status_led;
