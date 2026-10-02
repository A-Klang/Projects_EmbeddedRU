#include <unity.h>
#include <controller.h>
#include <controllers.h>
#include <Arduino.h>

// Assert PI controller is used, by checking what on_step update call uses
// Assert P controller is not used
void test_normal_flow(void) {
    PI_controller pi;
    
    //TEST_ASSERT_DOUBLE_WITHIN(0.01, 40.04, pi.update(41, 40));
}



// Assert P controller is used
// Assert PI controller is not used

// Is the steady-state error ~0?


int main()
{
 // NOTE!!! Wait for >2 secs
 // if board doesn't support software reset via Serial.DTR/RTS

 UNITY_BEGIN(); // IMPORTANT LINE!
 RUN_TEST(test_normal_flow);


 UNITY_END(); // stop unit testing
}