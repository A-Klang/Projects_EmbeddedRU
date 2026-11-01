#include <gpiod.h>
#include <stdio.h>

int main(void)
{
    unsigned int input_pin = 23;
    unsigned int output_pin = 21; 
    struct gpiod_chip *chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) { perror("gpiod_chip_open"); return 1; }

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(settings, GPIOD_LINE_EDGE_BOTH);

    struct gpiod_line_settings *settings_out = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings_out, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings_out, GPIOD_LINE_VALUE_INACTIVE);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(line_cfg, &input_pin, 1, settings);
    gpiod_line_config_add_line_settings(line_cfg, &output_pin, 1, settings_out);

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "input_pin-watch");


    struct gpiod_line_request *request =
    gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings_out);
    gpiod_chip_close(chip);
    if (!request) { perror("gpiod_chip_request_lines"); return 1; }

    struct gpiod_edge_event_buffer *buf = gpiod_edge_event_buffer_new(16);

    for (;;) {
    // Sleeps (no CPU use) until the kernel has queued an event
    int n = gpiod_line_request_read_edge_events(request, buf, 16);
    if (n < 0) { perror("read_edge_events"); break; }

    for (int i = 0; i < n; i++) {
        struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(buf, i);
        int rising = gpiod_edge_event_get_event_type(ev) == GPIOD_EDGE_EVENT_RISING_EDGE;
        enum gpiod_line_value x = gpiod_line_request_get_value(request, input_pin);
        gpiod_line_request_set_value(request, output_pin, x);
    }
    }

    gpiod_edge_event_buffer_free(buf);
    gpiod_line_request_release(request);
    return 0;
}