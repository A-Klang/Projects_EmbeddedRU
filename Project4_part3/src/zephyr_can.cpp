#include <zephyr_can.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <string.h>

CAN_MSGQ_DEFINE(can_rx_msgq, 16);

static const struct device *can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

static bool add_filter(uint16_t id) {
    struct can_filter filter = {};
    filter.id = id;
    filter.mask = CAN_STD_ID_MASK;
    return can_add_rx_filter_msgq(can_dev, &can_rx_msgq, &filter) >= 0;
}

bool ZephyrCan::init() {
    if (!device_is_ready(can_dev)) return false;
    if (can_set_bitrate(can_dev, 125000) != 0) return false;
    if (can_start(can_dev) != 0) return false;
    return add_filter(0x000) && add_filter(0x601) && add_filter(0x201);
}

bool ZephyrCan::send(const CanFrame &frame) {
    struct can_frame f = {};
    f.id = frame.id;
    f.dlc = frame.dlc;
    memcpy(f.data, frame.data, frame.dlc);
    return can_send(can_dev, &f, K_MSEC(10), NULL, NULL) == 0;
}

bool ZephyrCan::receive(CanFrame &frame) {
    struct can_frame f;
    if (k_msgq_get(&can_rx_msgq, &f, K_FOREVER) != 0) return false;
    frame.id = f.id;
    frame.dlc = f.dlc;
    memcpy(frame.data, f.data, f.dlc);
    return true;
}
