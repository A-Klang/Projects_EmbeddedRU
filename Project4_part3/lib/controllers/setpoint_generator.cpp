#include <setpoint_generator.h>

double SetpointGenerator::update(double target) {
    // Speeding up means moving away from zero, slowing down means moving towards it
    bool speeding_up = (ref >= 0 && target > ref) || (ref <= 0 && target < ref);
    double limit = speeding_up ? acc : dec;

    if (limit == 0) { // 0 = no limit => true step
        ref = target;
        return ref;
    }

    double step = limit * dt;
    double old_ref = ref;
    if (target > ref) {
        ref = (target - ref <= step) ? target : ref + step;
    } else {
        ref = (ref - target <= step) ? target : ref - step;
    }

    // Slowing down stops at zero, from the next update the other direction counts as speeding up
    if (!speeding_up && ((old_ref > 0 && ref < 0) || (old_ref < 0 && ref > 0))) {
        ref = 0;
    }
    return ref;
}

void SetpointGenerator::reset() {
    ref = 0;
}