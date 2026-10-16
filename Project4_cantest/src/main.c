#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>

CAN_MSGQ_DEFINE(rx_msgq, 10);

int main(void)
{
    const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
    if (!device_is_ready(dev)) { printk("CAN device not ready\n"); return 0; }
    if (can_set_bitrate(dev, 125000) != 0) { printk("Bitrate error\n"); return 0; }
    if (can_start(dev) != 0) { printk("Start error\n"); return 0; }

    const struct can_filter filter = { .flags = 0, .id = 0x321, .mask = CAN_STD_ID_MASK };
    if (can_add_rx_filter_msgq(dev, &rx_msgq, &filter) < 0) { printk("Filter error\n"); }

    struct can_frame tx = { .flags = 0, .id = 0x456, .dlc = 4, .data = {1, 2, 3, 4, 5, 6, 7, 8} };
    struct can_frame rx;

    while (1) {
        int err = can_send(dev, &tx, K_MSEC(100), NULL, NULL);
        if (err != 0) printk("Sending failed (err %d)\n", err);

        while (k_msgq_get(&rx_msgq, &rx, K_NO_WAIT) == 0) {
            printk("RX 0x%03X [%d]", rx.id, rx.dlc);
            for (int i = 0; i < rx.dlc; i++) printk(" %02X", rx.data[i]);
            printk("\n");
        }
        k_msleep(100);
    }
}
