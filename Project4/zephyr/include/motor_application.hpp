#pragma once

#include "canopen.hpp"
#include "state_machine.hpp"

#include <atomic>
#include <cstdint>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

class EncoderHardware {
public:
    bool init();
    std::int32_t position() const;
    std::int32_t sample_speed_rpm();
    void reset();

private:
    static void interrupt_handler(const device* port,
                                  gpio_callback* callback,
                                  gpio_port_pins_t pins);
    void on_edge();

    gpio_callback callback_{};
    std::atomic<std::int32_t> position_{0};
    std::int32_t history_[10]{};
    std::uint8_t history_index_ = 0;
};

class MotorHardware {
public:
    bool init();
    void set_output(double command);
    void coast();
    void brake();
    bool fault_active() const;
    void set_led(bool on);
    void toggle_led();
    std::int16_t duty_tenths_percent() const;

private:
    double applied_command_ = 0.0;
};

class MotorApplication final : public INmtHooks {
public:
    explicit MotorApplication(canopen::ProcessData& process_data);

    bool init();
    void control_update();
    bool fault_active() const;

    void on_state_changed(canopen::NmtState state) override;
    void on_reset_node() override;
    void on_reset_communication() override;

private:
    void update_state_led();

    canopen::ProcessData& process_data_;
    EncoderHardware encoder_;
    MotorHardware motor_;
    canopen::NmtState state_ = canopen::NmtState::Initializing;
    std::uint16_t led_ticks_ = 0;
};
