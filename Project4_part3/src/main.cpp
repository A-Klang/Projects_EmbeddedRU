#include <encoder.h>
#include <context.h>
#include <nmt_states.h>
#include <zephyr/kernel.h>
#include <zephyr_can.h>
#include <canopen_node.h>
#include <motor_app.h>

#define CONTROLLER_TASK_PRIORITY 1
K_MUTEX_DEFINE(app_mutex);
K_SEM_DEFINE(control_sem, 0, 1); // given by the 1 ms timer ISR, taken by the control thread


ZephyrCan can;
Context context(&initialization);
MotorApp app(context);
CanOpenNode node(can, app, 1);

void tpdo_handler(struct k_work *work)
{
  node.on_control_tick();
}
K_WORK_DEFINE(tpdo_work, tpdo_handler);

void controller_task(void*, void*, void*)
{
  while(1) {
    k_sem_take(&control_sem, K_FOREVER);

    k_mutex_lock(&app_mutex, K_FOREVER);
    if (enc.has_fault()) {
      context.fault();
    }
    context.step();
    k_mutex_unlock(&app_mutex);

    if (enc.counter >= 10) {
      k_work_submit(&tpdo_work);
      enc.counter = 0;
    }
  }
}


K_THREAD_DEFINE(controller_task_id, 1024, controller_task, NULL, NULL, NULL, CONTROLLER_TASK_PRIORITY, 0, SYS_FOREVER_MS);


static void boot_up() { node.send_bootup(); }

int main() {
  enc.init();
  status_led.init();
  if (!can.init()) printk("CAN init failed\n");
  on_boot_up = boot_up;
  context.start();
  k_thread_start(controller_task_id);

  node.process();


  //return 0;
}

