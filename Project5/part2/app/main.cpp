#include <encoder.h>
#include <pwm.h>
#include <controllers.h>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) { printf("usage: %s <ref_rpm>\n", argv[0]); return 1; }
    double ref_speed = atof(argv[1]);

    Encoder enc;
    Pwm pwm;
    if (!enc.init()) { perror("open /dev/encoder"); return 1; }
    if (!pwm.init()) { perror("pwm init"); return 1; }

    auto last = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; i++) {
        usleep(10000);
        enc.sample_speed();
        pwm.set(pi.update(ref_speed, enc.get_speed()));

        auto now = std::chrono::steady_clock::now();
        float period_ms = std::chrono::duration<float, std::milli>(now - last).count();
        last = now;
        printf("%.1f,%.3f\n", enc.get_speed(), period_ms);
    }
    pwm.stop();
    return 0;
}
