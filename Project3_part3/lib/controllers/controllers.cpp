#include <controllers.h>

P_controller P;
PI_controller PI;
Controller* controller = &PI;

double P_controller::update(double ref, double actual) {
    return Kp * (ref - actual);
}

void P_controller::set_Kp(double K_p) {
    Kp = K_p;
}

double PI_controller::update(double ref, double actual) {
    //TODO:
    double E = 0;
    return Kp * ((ref - actual) + ((1/Ti)*E));
}

void PI_controller::set_Kp(double K_p) {
    Kp = K_p;
}

void PI_controller::set_Ti(double T_i) {
    Ti = T_i;
}