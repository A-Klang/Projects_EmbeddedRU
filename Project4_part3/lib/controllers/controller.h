#pragma once
#include <math.h>
#include <setpoint_generator.h>

class Controller {
    public:
        Controller(double Kp, double Ti) : Kp(Kp), Ti(Ti) {}
        virtual double update(double ref, double actual) = 0;
        virtual void reset() = 0;

        double Kp;
        double Ti;
};

extern SetpointGenerator setpoint;
