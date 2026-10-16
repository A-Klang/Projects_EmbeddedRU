#include "motor_application.hpp"

#include <controllers.h>

#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <algorithm>
#include <cmath>

LOG_MODULE_REGISTER(motor_application, LOG_LEVEL_INF);

namespace {

constexpr float kCountsPerRevolution = 2100.0F;
constexpr float kSpeedWindowMs = 10.0F;

const gpio_dt_spec c1 = GPIO_DT_SPEC_GET(DT_ALIAS(c1_pin), gpios);
const gpio_dt_spec c2 = GPIO_DT_SPEC_GET(DT_ALIAS(c2_pin), gpios);
const gpio_dt_spec fault = GPIO_DT_SPEC_GET(DT_ALIAS(flt_pin), gpios);
const gpio_dt_spec direction = GPIO_DT_SPEC_GET(DT_ALIAS(gpio_dir), gpios);
const gpio_dt_spec status_led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
const pwm_dt_spec motor_pwm = PWM_DT_SPEC_GET(DT_ALIAS(pwm_pin));

bool gpio_ready(const gpio_dt_spec& spec, const char* name) {
    if (!gpio_is_ready_dt(&spec)) {
        LOG_ERR("GPIO %s is not ready", name);
        return false;
    }
    return true;
}

}  // namespace

bool EncoderHardware::init() {
    if (!gpio_ready(c1, "C1") || !gpio_ready(c2, "C2")) {
        return false;
    }
    if (gpio_pin_configure_dt(&c1, GPIO_INPUT) != 0 ||
        gpio_pin_configure_dt(&c2, GPIO_INPUT) != 0) {
        LOG_ERR("Cannot configure encoder inputs");
        return false;
    }

    gpio_init_callback(&callback_, &EncoderHardware::interrupt_handler,
                       BIT(c1.pin));
    if (gpio_add_callback(c1.port, &callback_) != 0 ||
        gpio_pin_interrupt_configure_dt(&c1, GPIO_INT_EDGE_BOTH) != 0) {
        LOG_ERR("Cannot configure C1 interrupt");
        return false;
    }

    reset();
    return true;
}

std::int32_t EncoderHardware::position() const {
    return position_.load(std::memory_order_relaxed);
}

std::int32_t EncoderHardware::sample_speed_rpm() {
    const std::int32_t current = position();
    const std::int32_t delta = current - history_[history_index_];
    history_[history_index_] = current;
    history_index_ = (history_index_ + 1U) % 10U;

    const float rpm =
        (static_cast<float>(delta) / kSpeedWindowMs) /
        kCountsPerRevolution * 60000.0F;
    return static_cast<std::int32_t>(std::lround(rpm));
}

void EncoderHardware::reset() {
    position_.store(0, std::memory_order_relaxed);
    for (std::int32_t& sample : history_) {
        sample = 0;
    }
    history_index_ = 0;
}

void EncoderHardware::interrupt_handler(const device*,
                                        gpio_callback* callback,
                                        gpio_port_pins_t) {
    auto* encoder = CONTAINER_OF(callback, EncoderHardware, callback_);
    encoder->on_edge();
}

void EncoderHardware::on_edge() {
    const int c1_level = gpio_pin_get_dt(&c1);
    const int c2_level = gpio_pin_get_dt(&c2);
    if (c1_level < 0 || c2_level < 0) {
        return;
    }
    position_.fetch_add(c1_level == c2_level ? 1 : -1,
                        std::memory_order_relaxed);
}

bool MotorHardware::init() {
    if (!gpio_ready(fault, "FLT") || !gpio_ready(direction, "direction") ||
        !gpio_ready(status_led, "led0") || !pwm_is_ready_dt(&motor_pwm)) {
        LOG_ERR("Motor hardware is not ready");
        return false;
    }

    if (gpio_pin_configure_dt(&fault, GPIO_INPUT) != 0 ||
        gpio_pin_configure_dt(&direction, GPIO_OUTPUT_INACTIVE) != 0 ||
        gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE) != 0) {
        LOG_ERR("Cannot configure motor GPIO");
        return false;
    }

    coast();
    return true;
}

void MotorHardware::set_output(double command) {
    applied_command_ = command;
    if (applied_command_ > 255.0) {
        applied_command_ = 255.0;
    } else if (applied_command_ < -255.0) {
        applied_command_ = -255.0;
    }
    const double magnitude = std::fabs(applied_command_) / 255.0;

    std::uint32_t pulse = 0;
    if (applied_command_ >= 0.0) {
        gpio_pin_set_dt(&direction, 0);
        pulse = static_cast<std::uint32_t>(motor_pwm.period * magnitude);
    } else {
        gpio_pin_set_dt(&direction, 1);
        pulse = static_cast<std::uint32_t>(motor_pwm.period * (1.0 - magnitude));
    }
    pwm_set_pulse_dt(&motor_pwm, pulse);
}

void MotorHardware::coast() {
    applied_command_ = 0.0;
    gpio_pin_set_dt(&direction, 0);
    pwm_set_pulse_dt(&motor_pwm, 0);
}

void MotorHardware::brake() {
    applied_command_ = 0.0;
    gpio_pin_set_dt(&direction, 1);
    pwm_set_pulse_dt(&motor_pwm, motor_pwm.period);
}

bool MotorHardware::fault_active() const {
    return gpio_pin_get_dt(&fault) > 0;
}

void MotorHardware::set_led(bool on) {
    gpio_pin_set_dt(&status_led, on ? 1 : 0);
}

void MotorHardware::toggle_led() {
    gpio_pin_toggle_dt(&status_led);
}

std::int16_t MotorHardware::duty_tenths_percent() const {
    return static_cast<std::int16_t>(
        std::lround(applied_command_ * 1000.0 / 255.0));
}

MotorApplication::MotorApplication(canopen::ProcessData& process_data)
    : process_data_(process_data) {}

bool MotorApplication::init() {
    return encoder_.init() && motor_.init();
}

void MotorApplication::control_update() {
    process_data_.velocity_actual_rpm = encoder_.sample_speed_rpm();

    if (state_ == canopen::NmtState::Operational) {
        Controller* controller = process_data_.controller_type == 0 ?
                                     static_cast<Controller*>(&p) :
                                     static_cast<Controller*>(&pi);
        controller->Kp = process_data_.kp;
        if (process_data_.ti > 0.0F) {
            pi.Ti = process_data_.ti;
        }

        const double command = controller->update(
            static_cast<double>(process_data_.target_velocity_rpm),
            static_cast<double>(process_data_.velocity_actual_rpm));
        motor_.set_output(command);
        process_data_.output_duty_tenths_percent =
            motor_.duty_tenths_percent();
    } else {
        process_data_.output_duty_tenths_percent = 0;
    }

    update_state_led();
}

bool MotorApplication::fault_active() const {
    return motor_.fault_active();
}

void MotorApplication::on_state_changed(canopen::NmtState state) {
    state_ = state;
    led_ticks_ = 0;

    switch (state_) {
        case canopen::NmtState::Operational:
            p.reset();
            pi.reset();
            motor_.set_led(true);
            break;
        case canopen::NmtState::Stopped:
            motor_.brake();
            motor_.set_led(false);
            break;
        case canopen::NmtState::Initializing:
        case canopen::NmtState::PreOperational:
            motor_.coast();
            motor_.set_led(false);
            break;
    }

    LOG_INF("NMT state 0x%02x", static_cast<unsigned>(state_));
}

void MotorApplication::on_reset_node() {
    encoder_.reset();
    motor_.coast();
}

void MotorApplication::on_reset_communication() {
    motor_.coast();
}

void MotorApplication::update_state_led() {
    if (state_ == canopen::NmtState::PreOperational) {
        if (++led_ticks_ >= 500) {
            led_ticks_ = 0;
            motor_.toggle_led();
        }
    } else if (state_ == canopen::NmtState::Stopped) {
        if (++led_ticks_ >= 250) {
            led_ticks_ = 0;
            motor_.toggle_led();
        }
    }
}
