#include "canopen.hpp"
#include "state_machine.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

using canopen::CanFrame;

class FakeCan final : public canopen::ICan {
public:
    bool send(const CanFrame& frame) override {
        frames.push_back(frame);
        return true;
    }

    const CanFrame& last() const { return frames.back(); }
    void clear() { frames.clear(); }

    std::vector<CanFrame> frames;
};

class FakeHooks final : public INmtHooks {
public:
    void on_state_changed(canopen::NmtState state) override {
        last_state = state;
    }
    void on_reset_node() override { ++node_resets; }
    void on_reset_communication() override { ++communication_resets; }

    canopen::NmtState last_state = canopen::NmtState::Initializing;
    int node_resets = 0;
    int communication_resets = 0;
};

[[noreturn]] void fail(const char* expression, int line) {
    std::cerr << "FAIL line " << line << ": " << expression << '\n';
    std::exit(1);
}

#define CHECK(expression) \
    do {                  \
        if (!(expression)) fail(#expression, __LINE__); \
    } while (false)

CanFrame nmt(std::uint8_t command, std::uint8_t node_id = 1) {
    CanFrame frame{};
    frame.id = canopen::kCobNmt;
    frame.dlc = 2;
    frame.data[0] = command;
    frame.data[1] = node_id;
    return frame;
}

CanFrame sdo_read(std::uint16_t index) {
    CanFrame frame{};
    frame.id = canopen::kCobSdoRequest;
    frame.dlc = 8;
    frame.data[0] = 0x40;
    canopen::encode_u16(index, &frame.data[1]);
    return frame;
}

CanFrame sdo_write_u8(std::uint16_t index, std::uint8_t value) {
    CanFrame frame{};
    frame.id = canopen::kCobSdoRequest;
    frame.dlc = 8;
    frame.data[0] = 0x2f;
    canopen::encode_u16(index, &frame.data[1]);
    frame.data[4] = value;
    return frame;
}

CanFrame sdo_write_u32(std::uint16_t index, std::uint32_t value) {
    CanFrame frame{};
    frame.id = canopen::kCobSdoRequest;
    frame.dlc = 8;
    frame.data[0] = 0x23;
    canopen::encode_u16(index, &frame.data[1]);
    canopen::encode_u32(value, &frame.data[4]);
    return frame;
}

struct Fixture {
    Fixture()
        : nmt_machine(hooks),
          dictionary(data, nmt_machine),
          node(can, nmt_machine, dictionary, data, 2) {
        node.start();
    }

    FakeCan can;
    FakeHooks hooks;
    MotorStateMachine nmt_machine;
    canopen::ProcessData data;
    canopen::ObjectDictionary dictionary;
    canopen::CanopenNode node;
};

void test_bootup_and_nmt() {
    Fixture f;
    CHECK(f.can.frames.size() == 1);
    CHECK(f.can.last().id == 0x701);
    CHECK(f.can.last().dlc == 1);
    CHECK(f.can.last().data[0] == 0);
    CHECK(f.nmt_machine.state() == canopen::NmtState::PreOperational);

    f.node.process(nmt(0x01));
    CHECK(f.nmt_machine.state() == canopen::NmtState::Operational);
    f.node.process(nmt(0x80, 0));
    CHECK(f.nmt_machine.state() == canopen::NmtState::PreOperational);
    f.node.process(nmt(0x01, 2));
    CHECK(f.nmt_machine.state() == canopen::NmtState::PreOperational);
}

void test_sdo_upload_and_download() {
    Fixture f;
    f.can.clear();

    f.node.process(sdo_read(0x1000));
    CHECK(f.can.last().id == 0x581);
    CHECK(f.can.last().data[0] == 0x43);
    CHECK(canopen::decode_u16(&f.can.last().data[1]) == 0x1000);

    f.node.process(sdo_write_u8(0x2000, 0));
    CHECK(f.can.last().data[0] == 0x60);
    CHECK(f.data.controller_type == 0);

    f.node.process(sdo_write_u32(0x2001, 0x3f000000));
    CHECK(f.can.last().data[0] == 0x60);
    CHECK(std::fabs(f.data.kp - 0.5F) < 0.0001F);
}

void test_required_abort_codes() {
    Fixture f;
    f.can.clear();

    f.node.process(sdo_read(0x9999));
    CHECK(f.can.last().data[0] == 0x80);
    CHECK(canopen::decode_u32(&f.can.last().data[4]) == 0x06020000);

    f.node.process(nmt(0x01));
    f.node.process(sdo_write_u32(0x2001, 0x3f000000));
    CHECK(f.can.last().data[0] == 0x80);
    CHECK(canopen::decode_u32(&f.can.last().data[4]) == 0x08000022);
}

void test_rpdo_and_tpdo() {
    Fixture f;
    CanFrame rpdo{};
    rpdo.id = canopen::kCobRpdo1;
    rpdo.dlc = 4;
    canopen::encode_u32(1500, rpdo.data.data());

    f.node.process(rpdo);
    CHECK(f.data.target_velocity_rpm == 0);

    f.node.process(nmt(0x01));
    f.node.process(rpdo);
    CHECK(f.data.target_velocity_rpm == 1500);

    f.data.velocity_actual_rpm = -120;
    f.data.output_duty_tenths_percent = -250;
    f.can.clear();
    f.node.on_control_update();
    CHECK(f.can.frames.empty());
    f.node.on_control_update();
    CHECK(f.can.frames.size() == 1);
    CHECK(f.can.last().id == 0x181);
    CHECK(f.can.last().dlc == 6);
    CHECK(static_cast<std::int32_t>(
              canopen::decode_u32(f.can.last().data.data())) == -120);
    CHECK(static_cast<std::int16_t>(
              canopen::decode_u16(&f.can.last().data[4])) == -250);
}

void test_resets() {
    Fixture f;
    f.data.kp = 12.0F;
    f.data.target_velocity_rpm = 900;
    f.can.clear();

    f.node.process(nmt(0x82));
    CHECK(f.hooks.communication_resets == 1);
    CHECK(f.data.kp == 12.0F);
    CHECK(f.data.target_velocity_rpm == 0);
    CHECK(f.can.last().id == 0x701);

    f.data.kp = 12.0F;
    f.node.process(nmt(0x81));
    CHECK(f.hooks.node_resets == 1);
    CHECK(f.data.kp == 40.0F);
    CHECK(f.can.last().id == 0x701);
}

}  // namespace

int main() {
    test_bootup_and_nmt();
    test_sdo_upload_and_download();
    test_required_abort_codes();
    test_rpdo_and_tpdo();
    test_resets();
    std::cout << "PASS: CANopen host tests\n";
    return 0;
}
