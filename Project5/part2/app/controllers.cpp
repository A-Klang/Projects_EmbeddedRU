#include <controllers.h>

P_controller p(40);
PI_controller pi(5, 0.1, 0.01); // dt = 10 ms control period

double P_controller::update(double ref, double actual) {
    return Kp * (ref - actual);
}

void P_controller::reset() {
    return;
}

double PI_controller::update(double ref, double actual) {
    double e = ref - actual;
    double E_new = E + e * dt;
    double u = Kp * (e + E_new / Ti);

    if (u > 255) return 255;
    if (u < -255) return -255;

    E = E_new;
    return u;
}

void PI_controller::reset() {
    E = 0;
}