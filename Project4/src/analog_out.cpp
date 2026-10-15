#include <zephyr/drivers/pwm.h>
#include <analog_out.h>

static const struct pwm_dt_spec pwm_spec = PWM_DT_SPEC_GET(DT_ALIAS(pwm_pin));

Analog_out::Analog_out(int pin) : pin(pin) {}

void Analog_out :: init(int period_us) {
    period_ns = PWM_USEC(period_us);
    pwm_set_dt(&pwm_spec, period_ns, 0);
}
void Analog_out :: set(double duty_cycle){
    if (duty_cycle < 0) duty_cycle = 0; // Keep magnitude to 0 and 255
    if (duty_cycle > 255) duty_cycle = 255;
    uint32_t pulse = (uint32_t)(period_ns * duty_cycle / 255.0);
    pwm_set_dt(&pwm_spec, period_ns, pulse);
}