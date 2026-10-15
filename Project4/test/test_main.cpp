#include <unity.h>
#include <controller.h>
#include <controllers.h>
#include <Arduino.h>

// Kp = 40 and dt/Ti = 0.1 so the expected values can be checked by hand
// TDD "before" run (logs/part3_tests_before.png) used P_controller ctrl(40) here
PI_controller ctrl(40, 1, 0.1);

void test_integral_accumulates(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01, 88, ctrl.update(2,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 96, ctrl.update(2,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 104, ctrl.update(2,0));

    ctrl.reset();
}

void test_zero_err_holds(void) {
    ctrl.update(2,0);
    ctrl.update(2,0);
    ctrl.update(2,0);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 24, ctrl.update(3, 3));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 24, ctrl.update(3, 3));

    ctrl.reset();
}

void test_overshooting(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01, -88, ctrl.update(5,7));

    ctrl.reset();
}

void test_excessive_ref_speed(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01, 255, ctrl.update(100,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 255, ctrl.update(100,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 255, ctrl.update(100,0));

    TEST_ASSERT_FLOAT_WITHIN(0.01, -255, ctrl.update(-100,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, -255, ctrl.update(-100,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, -255, ctrl.update(-100,0));

    ctrl.reset();
}

void test_negative_speed(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01, -88, ctrl.update(-2,0));

    ctrl.reset();
}

void test_anti_windup(void) {
    for (int i = 0; i < 100; i++) ctrl.update(100, 0);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 88, ctrl.update(2, 0));
    ctrl.reset();

}


int main()
{
 UNITY_BEGIN();
 RUN_TEST(test_integral_accumulates);
 RUN_TEST(test_zero_err_holds);
 RUN_TEST(test_overshooting);
 RUN_TEST(test_negative_speed);
 RUN_TEST(test_excessive_ref_speed);
 RUN_TEST(test_anti_windup);

 UNITY_END(); 
}