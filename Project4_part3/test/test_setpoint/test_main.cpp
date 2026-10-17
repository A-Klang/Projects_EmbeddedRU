#include <unity.h>
#include <setpoint_generator.h>

// dt = 1ms, limits 1000 rpm/s, so ref moves 1 rpm per update
SetpointGenerator gen(0.001);

void setUp(void) {
    gen.reset();
    gen.acc = 1000;
    gen.dec = 1000;
}

void tearDown(void) {}

void test_limit_reached(void) {
    for (int i = 1; i <= 5; i++) {
        TEST_ASSERT_FLOAT_WITHIN(0.001, i, gen.update(5)); //no overshoot
    }
}

void test_target_reversed_during_ramp(void) {
    gen.dec = 2000; // 2 RPM per update when slowing down
    for (int i = 0; i < 10; i++) gen.update(40); // ramped up to 10 RPM
    TEST_ASSERT_FLOAT_WITHIN(0.001, 8, gen.update(0)); // now limited by the deceleration
    TEST_ASSERT_FLOAT_WITHIN(0.001, 6, gen.update(0));
}

void test_zero_limit_is_step(void) {
    gen.acc = 0;
    gen.dec = 0;
    TEST_ASSERT_FLOAT_WITHIN(0.001, 40, gen.update(40));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0, gen.update(0));
}

void test_deceleration_through_zero(void) {
    gen.dec = 2000; // 2RPM per update towards zero
    for (int i = 0; i < 3; i++) gen.update(3); // ramped up to 3 RPM
    TEST_ASSERT_FLOAT_WITHIN(0.001, 1, gen.update(-3));  // slowing down, deceleration limit
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0, gen.update(-3));  // stops at zero instead of crossing it
    TEST_ASSERT_FLOAT_WITHIN(0.001, -1, gen.update(-3)); // speeding up the other way, acceleration limit
    TEST_ASSERT_FLOAT_WITHIN(0.001, -2, gen.update(-3));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_limit_reached);
    RUN_TEST(test_target_reversed_during_ramp);
    RUN_TEST(test_zero_limit_is_step);
    RUN_TEST(test_deceleration_through_zero);
    return UNITY_END();
}