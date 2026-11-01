#include <gpiod.h>  // libgpiod v2 API
#include <stdio.h>  // perror()
#include <unistd.h> // usleep()

int main(void)
{
    const char *chip_path = "/dev/gpiochip0";
    unsigned int output_pin = 21; // line offset on the chip
    unsigned int input_pin = 23;

    struct gpiod_chip *chip = gpiod_chip_open(chip_path);
    if (!chip) { perror("gpiod_chip_open"); return 1; }

    // 1. Line settings: output, initially inactive
    struct gpiod_line_settings *settings_out = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings_out, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings_out, GPIOD_LINE_VALUE_INACTIVE);

    struct gpiod_line_settings *settings_in = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings_in, GPIOD_LINE_DIRECTION_INPUT);

    // 2. Line config: apply the settings to offset 21
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(line_cfg, &output_pin, 1, settings_out);
    gpiod_line_config_add_line_settings(line_cfg, &input_pin, 1, settings_in);


    // 3. Request config: consumer name shown by gpioinfo
    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "example1");

    // 4. Request the line - the request outlives the chip handle
    struct gpiod_line_request *request =
    gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings_out);
    gpiod_line_settings_free(settings_in);
    gpiod_chip_close(chip);
    if (!request) { perror("gpiod_chip_request_lines"); return 1; }

    while(true)
    {
    enum gpiod_line_value x = gpiod_line_request_get_value(request, input_pin);
    gpiod_line_request_set_value(request, output_pin, x);
    usleep(800); // 0.8ms
    }

    return 0;
}