#include <pwm.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#define PWM_CHIP_PATH "/sys/class/pwm/pwmchip0/" // Adjust for your specific PWM chip

static bool write_attr(const char *attr, const char *value) {
    int fd;
    char path[50];

    snprintf(path, sizeof(path), "%s/%s", PWM_CHIP_PATH, attr);
    fd = open(path, O_WRONLY);
    if (fd < 0) { perror(path); return false; }
    if (write(fd, value, strlen(value)) < 0) perror(path);
    close(fd);
    return true;
}

bool Pwm::init() {
    char period[16];
    snprintf(period, sizeof(period), "%d", period_ns);

    write_attr("export", "0");
    write_attr("export", "1");

    return write_attr("pwm0/period", period)
        && write_attr("pwm0/duty_cycle", "0")
        && write_attr("pwm0/enable", "1")
        && write_attr("pwm1/period", period)
        && write_attr("pwm1/duty_cycle", "0")
        && write_attr("pwm1/enable", "1");
}

void Pwm::set(double u) {
    int duty_ns = (int)(fabs(u) / 255.0 * period_ns);
    if (duty_ns > period_ns) duty_ns = period_ns;

    char duty[16];
    snprintf(duty, sizeof(duty), "%d", duty_ns);

    write_attr(u >= 0 ? "pwm1/duty_cycle" : "pwm0/duty_cycle", "0");
    write_attr(u >= 0 ? "pwm0/duty_cycle" : "pwm1/duty_cycle", duty);
}

void Pwm::stop() {
    write_attr("pwm0/enable", "0");
    write_attr("pwm1/enable", "0");
    write_attr("unexport", "0");
    write_attr("unexport", "1");
}
