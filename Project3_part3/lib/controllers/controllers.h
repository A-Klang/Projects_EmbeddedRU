#pragma once
#include <controller.h>

class P_controller : public Controller {
    public:
        double update(double ref, double actual) override;
        void set_Kp(double K_p) override;
};

class PI_controller : public Controller {
    public:
        double update(double ref, double actual) override;
        void set_Kp(double K_p) override;
        void set_Ti(double T_i) override;
};

extern P_controller P;
extern PI_controller PI;
extern Controller* controller;
