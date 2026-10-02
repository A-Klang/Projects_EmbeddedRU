#pragma once

class Controller {
    public:
        virtual double update(double ref, double actual);
};