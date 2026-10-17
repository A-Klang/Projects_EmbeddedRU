#include <digital_in.h>
#include <zephyr/drivers/gpio.h>

static const struct gpio_dt_spec c1_spec  = GPIO_DT_SPEC_GET(DT_ALIAS(c1_pin), gpios);
static const struct gpio_dt_spec c2_spec  = GPIO_DT_SPEC_GET(DT_ALIAS(c2_pin), gpios);
static const struct gpio_dt_spec flt_spec = GPIO_DT_SPEC_GET(DT_ALIAS(flt_pin), gpios);

static const struct gpio_dt_spec* lookup(uint8_t id) {
    switch (id) {
        case 4: return &c1_spec;
        case 3: return &c2_spec;
        case 2: return &flt_spec;
        default: return nullptr;
    }
}


Digital_in::Digital_in(uint8_t pin) : id(pin) {}


void Digital_in::init() {
    const struct gpio_dt_spec* spec = lookup(id);
    if (spec == nullptr) return;

    if (!gpio_is_ready_dt(spec)) {
        return;
    }
    if (gpio_pin_configure_dt(spec, GPIO_INPUT) < 0) {
        return;
    }
}

bool Digital_in::is_hi() {
    const struct gpio_dt_spec* spec = lookup(id);
    return spec != nullptr && gpio_pin_get_dt(spec) == 1;
}

bool Digital_in::is_lo() {
    const struct gpio_dt_spec* spec = lookup(id);
    return spec != nullptr && gpio_pin_get_dt(spec) == 0;
}
