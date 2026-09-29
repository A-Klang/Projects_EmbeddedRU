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
};