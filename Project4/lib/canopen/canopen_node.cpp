#include <canopen_node.h>
#include <object_dictionary.h>

static const uint16_t COB_NMT = 0x000;
static const uint16_t COB_RPDO1 = 0x200;  // + node id
static const uint16_t COB_SDO_TX = 0x580; // + node id, response from the node
static const uint16_t COB_SDO_RX = 0x600; // + node id, request from the client
static const uint16_t COB_BOOTUP = 0x700; // + node id

// CANopen sends multi-byte values little-endian, least significant byte first
static uint16_t get_le16(const uint8_t *p) { return p[0] | (p[1] << 8); }
static uint32_t get_le32(const uint8_t *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
static void put_le16(uint8_t *p, uint16_t v) { p[0] = v; p[1] = v >> 8; }
static void put_le32(uint8_t *p, uint32_t v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; }

static const uint16_t COB_TPDO1 = 0x180; // + node id
static const uint8_t TPDO1_EVERY = 10;   // control updates per TPDO1, 10 ms = 100 Hz



void CanOpenNode::process() {
    CanFrame f;
    while (can_.receive(f)) {
        if (f.id == COB_NMT) handle_nmt(f);
        else if (f.id == COB_SDO_RX + node_id_) handle_sdo(f);
        else if (f.id == COB_RPDO1 + node_id_) handle_rpdo(f);

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

void CanOpenNode::handle_sdo(const CanFrame &f) {
    NmtState s = app_.nmt_state();
    if (s != NmtState::PreOperational && s != NmtState::Operational) return; // no SDO in Stopped
    if (f.dlc < 8) return;

    uint8_t cmd = f.data[0];
    uint16_t index = get_le16(&f.data[1]);

    CanFrame r = {};
    r.id = COB_SDO_TX + node_id_;
    r.dlc = 8;
    put_le16(&r.data[1], index);
    r.data[3] = f.data[3]; // sub-index, echoed back

    uint32_t abort;
    if (cmd == 0x40) { // upload (read)
        uint32_t value = 0;
        uint8_t size = 4;
        abort = od_read(app_, index, value, size);
        r.data[0] = 0x43 | ((4 - size) << 2); // expedited, size indicated, n = unused bytes
        put_le32(&r.data[4], value);
    }
    else { // expedited download (write), 0x23 / 0x27 / 0x2B / 0x2F
        abort = od_write(app_, index, get_le32(&f.data[4]));
        r.data[0] = 0x60;
    }

    if (abort != SDO_OK) {
        r.data[0] = 0x80;
        put_le32(&r.data[4], abort);
    }
    can_.send(r);
}

void CanOpenNode::handle_rpdo(const CanFrame &f) {
    if (app_.nmt_state() != NmtState::Operational) return; // PDOs only in Operational
    if (f.dlc < 4) return;
    od_write(app_, 0x60FF, get_le32(&f.data[0])); // RPDO1 bytes 0-3 are mapped to the target velocity
}

void CanOpenNode::on_control_tick() {
    if (app_.nmt_state() != NmtState::Operational) { tpdo_count_ = 0; return; } // PDOs only in Operational
    if (++tpdo_count_ < TPDO1_EVERY) return;
    tpdo_count_ = 0;

    uint32_t velocity, duty;
    uint8_t size;
    od_read(app_, 0x606C, velocity, size);
    od_read(app_, 0x2010, duty, size);

    CanFrame f = {};
    f.id = COB_TPDO1 + node_id_;
    f.dlc = 6;
    put_le32(&f.data[0], velocity); // bytes 0-3, velocity actual value
    put_le16(&f.data[4], duty);     // bytes 4-5, output duty cycle
    can_.send(f);
}
