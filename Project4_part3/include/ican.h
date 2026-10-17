#pragma once
#include <stdint.h>

struct CanFrame {
    uint16_t id;
    uint8_t dlc;
    uint8_t data[8];
};

class ICan {
public:
    virtual bool send(const CanFrame &frame) = 0;
    virtual bool receive(CanFrame &frame) = 0;
    virtual ~ICan() {}
};