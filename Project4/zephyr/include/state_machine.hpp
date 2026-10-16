#pragma once

#include "canopen.hpp"

class INmtHooks {
public:
    virtual ~INmtHooks() = default;
    virtual void on_state_changed(canopen::NmtState state) = 0;
    virtual void on_reset_node() = 0;
    virtual void on_reset_communication() = 0;
};

class MotorStateMachine final : public canopen::INmtStateMachine {
public:
    explicit MotorStateMachine(INmtHooks& hooks);

    void start() override;
    bool handle_command(std::uint8_t command) override;
    void fault() override;
    canopen::NmtState state() const override;

private:
    void transition_to(canopen::NmtState next);

    INmtHooks& hooks_;
    canopen::NmtState state_ = canopen::NmtState::Initializing;
};
