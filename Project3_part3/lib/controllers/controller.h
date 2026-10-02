#pragma once

class Controller {
    public:
        virtual double update(double ref, double actual);
        virtual void set_Kp(double K_p);
        virtual void set_Ti(double T_i);

        double Kp = 40;
        double Ti = 1;
};