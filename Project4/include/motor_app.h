#pragma once
#include <node_app.h>
#include <context.h>

class MotorApp : public NodeApp {
public:
    MotorApp(Context &context) : context_(context) {}
    NmtState nmt_state() override;
    void nmt_start() override;
    void nmt_stop() override;
    void nmt_preoperational() override;
    void nmt_reset_node() override;
    void nmt_reset_communication() override;

private:
    Context &context_;
};
