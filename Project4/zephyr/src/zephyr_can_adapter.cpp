#include "zephyr_can_adapter.hpp"

#include <zephyr/devicetree.h>
#include <zephyr/drivers/can.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <cstring>

LOG_MODULE_REGISTER(can_adapter, LOG_LEVEL_INF);

CAN_MSGQ_DEFINE(can_rx_queue, 32);

bool ZephyrCanAdapter::init(std::uint32_t bitrate) {
    device_ = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
    if (!device_is_ready(device_)) {
        LOG_ERR("CAN device is not ready");
        return false;
    }

    int error = can_set_bitrate(device_, bitrate);
    if (error != 0) {
        LOG_ERR("Cannot set CAN bitrate (%d)", error);
        return false;
    }

    error = can_start(device_);
    if (error != 0) {
        LOG_ERR("Cannot start CAN controller (%d)", error);
        return false;
    }

    if (!add_filter(canopen::kCobNmt) ||
        !add_filter(canopen::kCobSdoRequest) ||
        !add_filter(canopen::kCobRpdo1)) {
        return false;
    }

    LOG_INF("CAN started at %u bit/s", bitrate);
    return true;
}

bool ZephyrCanAdapter::send(const canopen::CanFrame& source) {
    if (device_ == nullptr || source.dlc > 8) {
        return false;
    }

    can_frame frame{};
    frame.id = source.id;
    frame.dlc = source.dlc;
    frame.flags = 0;
    std::memcpy(frame.data, source.data.data(), source.dlc);

    const int error = can_send(device_, &frame, K_MSEC(20), nullptr, nullptr);
    if (error != 0) {
        LOG_ERR("CAN send failed: ID 0x%03x (%d)", source.id, error);
        return false;
    }
    return true;
}

bool ZephyrCanAdapter::receive(canopen::CanFrame& destination) {
    can_frame frame{};
    if (k_msgq_get(&can_rx_queue, &frame, K_NO_WAIT) != 0) {
        return false;
    }

    destination = {};
    destination.id = frame.id;
    destination.dlc = frame.dlc;
    std::memcpy(destination.data.data(), frame.data, frame.dlc);
    return true;
}

bool ZephyrCanAdapter::add_filter(std::uint32_t id) {
    can_filter filter{};
    filter.id = id;
    filter.mask = CAN_STD_ID_MASK;
    filter.flags = 0;

    const int filter_id =
        can_add_rx_filter_msgq(device_, &can_rx_queue, &filter);
    if (filter_id < 0) {
        LOG_ERR("Cannot add CAN filter 0x%03x (%d)", id, filter_id);
        return false;
    }
    return true;
}
