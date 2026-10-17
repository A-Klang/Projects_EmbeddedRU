#pragma once
class Context;

class State {
    public:
        Context* context_ = nullptr;
        virtual void on_entry() {}
        virtual void on_exit() {}
        virtual void on_step() {}
        virtual void on_reset() {}
        virtual void on_fault() {}
        virtual void on_set_operational() {}
        virtual void on_set_preoperational() {}
        virtual void on_set_Kp(double) {};
        virtual void on_set_Ti(double) {};
        virtual void on_set_control_law(char) {};
};