#pragma once

class Pwm {
    public:
        bool init();
        void set(double u); // controller output, -255 to 255
        void stop();

    private:
        static constexpr int period_ns = 1000000; // 1 ms, as in L5.4
};
