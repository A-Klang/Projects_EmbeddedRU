#pragma once
#include <controller.h>

class P_controller : public Controller {
    public:
        double update(double ref, double actual) override;
};

class PI_controller : public Controller {
    public:
        double update(double ref, double actual) override;
};
