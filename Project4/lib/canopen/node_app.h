#pragma once
#include <stdint.h>

enum class NmtState { Initialization, PreOperational, Operational, Stopped };

class NodeApp {
public:
    virtual NmtState nmt_state() = 0;
    virtual void nmt_start() = 0;
    virtual void nmt_stop() = 0;
    virtual void nmt_preoperational() = 0;
    virtual void nmt_reset_node() = 0;
    virtual void nmt_reset_communication() = 0;
    virtual ~NodeApp() {}
};