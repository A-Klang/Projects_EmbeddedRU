#include "canopen.hpp"
#include "motor_application.hpp"
#include "state_machine.hpp"
#include "zephyr_can_adapter.hpp"

#include <atomic>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(project4, LOG_LEVEL_INF);

namespace {

constexpr std::uint32_t kCanBitrate = 125000;
constexpr std::uint16_t kTpdoDivider = 20;  // 1 kHz / 20 = 50 Hz

std::atomic<std::uint32_t> pending_control_updates{0};

void control_timer_expired(k_timer*) {
    pending_control_updates.fetch_add(1, std::memory_order_relaxed);
}

K_TIMER_DEFINE(control_timer, control_timer_expired, nullptr);

}  // namespace

int main() {
    canopen::ProcessData process_data{};
    MotorApplication application(process_data);
    if (!application.init()) {
        LOG_ERR("Motor application initialization failed");
        return 0;
    }

    ZephyrCanAdapter can;
    if (!can.init(kCanBitrate)) {
        LOG_ERR("CAN initialization failed");
        return 0;
    }

    MotorStateMachine nmt(application);
    canopen::ObjectDictionary dictionary(process_data, nmt);
    canopen::CanopenNode node(can, nmt, dictionary, process_data,
                              kTpdoDivider);

    node.start();
    k_timer_start(&control_timer, K_MSEC(1), K_MSEC(1));
    LOG_INF("CANopen node %u ready", canopen::kNodeId);

    while (true) {
        const std::uint32_t updates =
            pending_control_updates.exchange(0, std::memory_order_relaxed);
        for (std::uint32_t i = 0; i < updates; ++i) {
            application.control_update();
            node.on_control_update();
        }

        canopen::CanFrame frame{};
        while (can.receive(frame)) {
            node.process(frame);
        }

        if (application.fault_active()) {
            nmt.fault();
        }

        k_msleep(1);
    }

    return 0;
}
