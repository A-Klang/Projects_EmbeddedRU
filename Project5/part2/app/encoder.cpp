#include <encoder.h>
#include <fcntl.h>
#include <unistd.h>


bool Encoder::init() {
    fd = open("/dev/encoder", O_RDONLY);
    if (fd < 0) return false;
    last_pos = read_pos();
    last_time = std::chrono::steady_clock::now();
    return true;
}

int Encoder::read_pos() {
    int value = last_pos; // keeps the last value if the read fails
    read(fd, &value, sizeof(value));
    return value;
}

void Encoder::sample_speed() {
    int now_pos = read_pos();
    auto now = std::chrono::steady_clock::now();

    float dt_s = std::chrono::duration<float>(now - last_time).count();
    speed_rpm = ((now_pos - last_pos) / counts_per_rev) / dt_s * 60.0f;

    last_pos = now_pos;
    last_time = now;
}


float Encoder::get_speed() {
    return speed_rpm;
}
