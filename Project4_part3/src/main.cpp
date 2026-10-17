#include <encoder.h>
#include <context.h>
#include <nmt_states.h>
#include <zephyr/kernel.h>
#include <zephyr_can.h>
#include <canopen_node.h>
#include <motor_app.h>

ZephyrCan can;
Context context(&initialization);
MotorApp app(context);
CanOpenNode node(can, app, 1);

static void boot_up() { node.send_bootup(); }

int main() {
  enc.init();
  status_led.init();
  if (!can.init()) printk("CAN init failed\n");
  on_boot_up = boot_up;
  context.start();

  while (1) {
    node.process();

    if (enc.has_fault()) {
      context.fault();
    }

    if (enc.control_tick) {
      enc.control_tick = false;
      context.step();
      node.on_control_tick();
    }
  }

  return 0;
}
