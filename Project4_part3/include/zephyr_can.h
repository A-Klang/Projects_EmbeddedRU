#pragma once
#include <ican.h>

class ZephyrCan : public ICan {
public:
    bool init();
    bool send(const CanFrame &frame) override;
    bool receive(CanFrame &frame) override;
};
