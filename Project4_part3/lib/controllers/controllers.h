#pragma once
#include <controller.h>

class P_controller : public Controller {
    public:
        P_controller(double Kp) : Controller(Kp, 0) {}
        double update(double ref, double actual) override;
        void reset() override;
};

class PI_controller : public Controller {
    public:
        PI_controller(double Kp, double Ti, double dt) : Controller(Kp, Ti), dt(dt) {}
        double update(double ref, double actual) override;
        void reset() override;

    private:
        double E = 0;
        double dt;
};

extern P_controller p;
extern PI_controller pi;
