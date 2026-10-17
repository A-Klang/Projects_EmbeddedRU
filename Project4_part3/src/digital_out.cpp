#include <digital_out.h>
#include <zephyr/drivers/gpio.h>

static const struct gpio_dt_spec dir_spec  = GPIO_DT_SPEC_GET(DT_ALIAS(gpio_dir), gpios);
static const struct gpio_dt_spec led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);


static const struct gpio_dt_spec* lookup(uint8_t id) {
    switch (id) {
        case 1: return &dir_spec;
        case 5: return &led_spec;
        default: return nullptr;
    }
}

Digital_out::Digital_out(uint8_t pin) 
: id(pin) {}

void Digital_out::init() {
    const struct gpio_dt_spec* spec = lookup(id);
    if (spec == nullptr) return;
    gpio_pin_configure_dt(spec, GPIO_OUTPUT_INACTIVE);
}

void Digital_out::set_hi() {
    const struct gpio_dt_spec* spec = lookup(id);
    if (spec != nullptr) gpio_pin_set_dt(spec, 1);

}

void Digital_out::set_lo() {
    const struct gpio_dt_spec* spec = lookup(id);
    if (spec != nullptr) gpio_pin_set_dt(spec, 0);
}

void Digital_out::toggle() {
    const struct gpio_dt_spec* spec = lookup(id);
    if (spec != nullptr) gpio_pin_toggle_dt(spec);
}