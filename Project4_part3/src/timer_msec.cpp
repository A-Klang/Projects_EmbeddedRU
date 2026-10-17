#include <zephyr/kernel.h>
#include <timer_msec.h>

void timer_msec_tick();

static void expiry(struct k_timer *t) {
    timer_msec_tick();
}
K_TIMER_DEFINE(tick_timer, expiry, NULL);

    
Timer_msec::Timer_msec(){}

void Timer_msec:: init(int period_ms, int duty_cycle){
    (void)duty_cycle;
    k_timer_start(&tick_timer, K_MSEC(period_ms), K_MSEC(period_ms));
}

void Timer_msec::set_duty_cycle(int duty_cycle){
    (void)duty_cycle;
}