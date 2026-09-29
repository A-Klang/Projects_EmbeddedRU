#pragma once

#include <digital_in.h>
#include <digital_out.h>

class P_controller
{
    public:
        P_controller(double Kp);
        double update(double ref, double actual);
        void set_Kp(double Kp);

    private:
        double K_p;
};

extern P_controller P_cont;