#include <controllers.h>

Controller controller;

double P_controller::update(double ref, double actual) {
    return K_p * (ref - actual);
}

double PI_controller::update(double ref, double actual) {
    //TODO:
    E = 0;
    return K_p * ((ref - actual) + ((1/T_i)*E));
}