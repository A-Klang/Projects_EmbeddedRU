#pragma once

#include <digital_in.h>
#include <digital_out.h>

class P_controller
{
    public:
        P_controller(double Kp);
        void update(double ref, double actual);
        double pwm_value;


    private:
        double K_p;
};

// The single p_controller instance. Defined in p_controller.cpp
extern P_controller P_cont;