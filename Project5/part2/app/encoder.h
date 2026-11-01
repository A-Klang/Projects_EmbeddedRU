#pragma once
#include <chrono>

class Encoder
{
    public:
        bool init();
        void sample_speed();
        float get_speed();


    private:
        int read_pos();
        static constexpr float counts_per_rev = 2100.0;
        float speed_rpm;

        int fd = -1;
        int last_pos = 0;
        std::chrono::steady_clock::time_point last_time;
};

extern Encoder enc;