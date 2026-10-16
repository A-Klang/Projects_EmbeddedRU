#include <canopen_node.h>

static const uint16_t COB_NMT = 0x000;
static const uint16_t COB_BOOTUP = 0x700; // + node id

void CanOpenNode::process() {
    CanFrame f;
    while (can_.receive(f)) {
        if (f.id == COB_NMT) handle_nmt(f);
    }
}

void CanOpenNode::handle_nmt(const CanFrame &f) {
    if (f.dlc < 2) return;
    uint8_t node = f.data[1];
    if (node != 0 && node != node_id_) return; // 0 = all nodes
    switch (f.data[0]) {
        case 0x01: app_.nmt_start(); break;
        case 0x02: app_.nmt_stop(); break;
        case 0x80: app_.nmt_preoperational(); break;
        case 0x81: app_.nmt_reset_node(); break;
        case 0x82: app_.nmt_reset_communication(); break;
    }
}

void CanOpenNode::send_bootup() {
    CanFrame f = {};
    f.id = COB_BOOTUP + node_id_;
    f.dlc = 1;
    f.data[0] = 0x00;
    can_.send(f);
}
