#pragma once
class Context;

class State {
    public:
        Context* context_ = nullptr;
        virtual void on_entry() {}
        virtual void on_exit() {}
        virtual void on_step() {}
        virtual void on_reset() {}
};