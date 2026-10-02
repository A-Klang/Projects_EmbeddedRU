#include <unity.h>
#include <controller.h>
#include <controllers.h>
#include <Arduino.h>

void test_integral_accumulates(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01, 88, controller->update(2,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 96, controller->update(2,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 104, controller->update(2,0));

    controller->reset();
}

void test_zero_err_holds(void) {
    controller->update(2,0);
    controller->update(2,0);
    controller->update(2,0);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 24, controller->update(3, 3));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 24, controller->update(3, 3));

    controller->reset();
}

void test_overshooting(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01, -88, controller->update(5,7));

    controller->reset();
}

void test_excessive_ref_speed(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01, 255, controller->update(100,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 255, controller->update(100,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 255, controller->update(100,0));

    TEST_ASSERT_FLOAT_WITHIN(0.01, -255, controller->update(-100,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, -255, controller->update(-100,0));
    TEST_ASSERT_FLOAT_WITHIN(0.01, -255, controller->update(-100,0));

    controller->reset();
}

void test_negative_speed(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01, -88, controller->update(-2,0));

    controller->reset();
}

void test_anti_windup(void) {
    for (int i = 0; i < 100; i++) controller->update(100, 0);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 88, controller->update(2, 0));
    controller->reset();

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