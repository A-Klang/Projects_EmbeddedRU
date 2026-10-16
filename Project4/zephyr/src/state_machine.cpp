#include "state_machine.hpp"

MotorStateMachine::MotorStateMachine(INmtHooks& hooks) : hooks_(hooks) {}

void MotorStateMachine::start() {
    state_ = canopen::NmtState::Initializing;
    hooks_.on_state_changed(state_);
    transition_to(canopen::NmtState::PreOperational);
}

bool MotorStateMachine::handle_command(std::uint8_t command_byte) {
    switch (static_cast<canopen::NmtCommand>(command_byte)) {
        case canopen::NmtCommand::Start:
            transition_to(canopen::NmtState::Operational);
            return false;
        case canopen::NmtCommand::Stop:
            transition_to(canopen::NmtState::Stopped);
            return false;
        case canopen::NmtCommand::EnterPreOperational:
            transition_to(canopen::NmtState::PreOperational);
            return false;
        case canopen::NmtCommand::ResetNode:
            transition_to(canopen::NmtState::Initializing);
            hooks_.on_reset_node();
            transition_to(canopen::NmtState::PreOperational);
            return true;
        case canopen::NmtCommand::ResetCommunication:
            transition_to(canopen::NmtState::Initializing);
            hooks_.on_reset_communication();
            transition_to(canopen::NmtState::PreOperational);
            return true;
        default:
            return false;
    }
}

void MotorStateMachine::fault() {
    transition_to(canopen::NmtState::Stopped);
}

canopen::NmtState MotorStateMachine::state() const {
    return state_;
}

void MotorStateMachine::transition_to(canopen::NmtState next) {
    if (state_ == next) {
        return;
    }
    state_ = next;
    hooks_.on_state_changed(state_);
}
