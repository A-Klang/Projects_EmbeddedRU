#pragma once
#include <state.h>

class Context {
    private:
        State* state_;
    public:
        Context(State* initial);
        void start();
        void transition_to(State* next);
        void step();
        void reset();
        void fault();
        void set_operational();
        void set_preoperational();
        void set_Kp(double K_p);
        void set_Ti(double T_i);
        void set_control_law(char law);
};