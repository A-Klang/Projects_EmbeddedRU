#pragma once
#include <ican.h>
#include <node_app.h>

class CanOpenNode {
public:
    CanOpenNode(ICan &can, NodeApp &app, uint8_t node_id) : can_(can), app_(app), node_id_(node_id) {}
    void process();     // handles all received frames, called from the main loop
    void send_bootup();

private:
    void handle_nmt(const CanFrame &f);
    void handle_sdo(const CanFrame &f);
    void handle_rpdo(const CanFrame &f);
    ICan &can_;
    NodeApp &app_;
    uint8_t node_id_;
};
