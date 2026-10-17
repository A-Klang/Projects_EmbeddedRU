#pragma once

class SetpointGenerator {
    public:
        SetpointGenerator(double dt) : dt(dt) {}
        double update(double target); // called once per control update, returns the new ref
        void reset();

        double acc = 0; // profile acc, 0 = no limit
        double dec = 0; // profile dec, 0 = no limit

    private:
        double dt;
        double ref = 0;
};