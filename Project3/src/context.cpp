#include <context.h>

Context::Context(State* initial) {
    state_ = initial;
}

void Context::start() {
    state_ -> context_ = this;
    state_ -> on_entry();
}

void Context::transition_to(State* next) {
    state_ -> on_exit();
    state_ = next;
    state_ -> context_ = this;
    state_ -> on_entry();
}

void Context::step() {
    state_ -> on_step();
}

void Context::reset() {
    state_ -> on_reset();
}

void Context::fault() {
    state_ -> on_fault();
}

void Context::set_operational() {
    state_ -> on_set_operational();
}
