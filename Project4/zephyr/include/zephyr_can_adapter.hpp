#pragma once

#include "canopen.hpp"

#include <zephyr/device.h>

class ZephyrCanAdapter final : public canopen::ICan {
public:
    bool init(std::uint32_t bitrate);
    bool send(const canopen::CanFrame& frame) override;
    bool receive(canopen::CanFrame& frame);

private:
    bool add_filter(std::uint32_t id);

    const device* device_ = nullptr;
};
