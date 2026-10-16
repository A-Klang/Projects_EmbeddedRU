#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <encoder.h>
#include <timer_msec.h>
#include <analog_out.h>

Encoder enc(4, 3, 0, 1, 2); // C1=D4/PB7, C2=D3/PB0, AIN1=D9/PA8, AIN2=D6/PB1, FLT=D5/PB6

Timer_msec timer;

static const struct gpio_dt_spec c1_int = GPIO_DT_SPEC_GET(DT_ALIAS(c1_pin), gpios);
static struct gpio_callback c1_cb_data;
static const struct gpio_dt_spec dbg = GPIO_DT_SPEC_GET(DT_ALIAS(dbg_pin), gpios); // D11, high while the callback runs

static void c1_isr(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins)
{
    gpio_pin_set_dt(&dbg, 1);
    enc.update();
    gpio_pin_set_dt(&dbg, 0);
}


Encoder::Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t AIN1_port, uint8_t AIN2_port, uint8_t flt_pin)
: pwm_pin(AIN1_port), dir_pin(AIN2_port), c1(c1_pin), c2(c2_pin), flt(flt_pin), last_c1(false),
pos(0), speed_rpm(0) {}


void Encoder::init() {
    timer.init(period_ms);
    c1.init();
    c2.init();
    flt.init();
    pwm_pin.init(); //Previously AIN1
    dir_pin.init(); //Previously AIN2
    last_c1 = c1.is_hi();
    pos = 0;
    counter = 0;

    gpio_pin_configure_dt(&dbg, GPIO_OUTPUT_INACTIVE);
    gpio_pin_interrupt_configure_dt(&c1_int, GPIO_INT_EDGE_BOTH);
    gpio_init_callback(&c1_cb_data, c1_isr, BIT(c1_int.pin));
    gpio_add_callback(c1_int.port, &c1_cb_data);

}

int Encoder::get_position() {
    return pos;
}

bool Encoder::has_fault() {
    return flt.is_lo();
}

void Encoder::brake() {
    dir_pin.set_hi();
    pwm_pin.set(255); //Set both pins high to actively brake
    pwm_value = 0;
}

float Encoder::get_speed() {
    float s;
    unsigned int key = irq_lock();
    s = speed_rpm;
    irq_unlock(key);

    return s;
}

void Encoder::stop_motor() {
    dir_pin.set_lo();
    pwm_pin.set(0); // Set both pins low to "coast"
    pwm_value = 0;
}

void Encoder::reset_position() {
    pos = 0;
    for (uint8_t i = 0; i < window_ticks; i++) {
        window_pos[i] = 0;
    }
}

void Encoder::update() {
    bool now_c1 = c1.is_hi();
    if (now_c1 == c2.is_hi())
        pos++;
    else
        pos--;
    last_c1 = now_c1;
}

void Encoder::sample_speed() {
    int now_pos = pos;
    int delta = now_pos - window_pos[tick]; // window_pos[tick] is the position `window_ticks`(10ms) ticks ago
    window_pos[tick] = now_pos;
    tick = (tick + 1) % window_ticks;
    speed_rpm = (((float)delta / (period_ms * window_ticks)) / counts_per_rev) * 60000.0f;
}

void timer_msec_tick() {
    enc.counter++;
    enc.ms_since_start++;
    enc.sample_speed();
    enc.control_tick = true;
}