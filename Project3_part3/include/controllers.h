#pragma once
#include <controller.h>

class P_controller : public Controller {
    public:
        double update(double ref, double actual) override;
        void reset() override;
};

class PI_controller : public Controller {
    public:
        double update(double ref, double actual) override;
        void reset() override;

    private:
        double E = 0;
        double dt = 0.001;
};

extern P_controller p;
extern PI_controller pi;
extern Controller* controller;
