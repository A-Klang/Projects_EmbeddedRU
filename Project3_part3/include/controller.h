#pragma once
#include <math.h>

class Controller {
    public:
        virtual double update(double ref, double actual);
        virtual void reset();

        double Kp = 80;
        double Ti = 0.1;
};