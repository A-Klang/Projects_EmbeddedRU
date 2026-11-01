#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("NN");
MODULE_DESCRIPTION("A simple LKM for gpio interrupt using pure descriptor API");

/** GPIO descriptor for our pin */
static struct gpio_desc *gpio_desc;

/** Variable contains IRQ number */
static unsigned int irq_number;
static struct gpio_desc *input_desc;
static struct gpio_desc *output_desc;

/**
 * @brief Interrupt service routine is called, when interrupt is triggered
 */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    gpiod_set_value(output_desc, gpiod_get_value(input_desc));
    return IRQ_HANDLED;
}

/**
 * @brief Platform driver probe function
 */
static int gpio_irq_probe(struct platform_device *pdev)
{
  int ret;
  /* Get GPIO descriptor using device-managed resource */
  gpio_desc = devm_gpiod_get(&pdev->dev, "interrupt", GPIOD_IN);
  if (IS_ERR(gpio_desc)) {
    printk("Error!\nCan not get GPIO descriptor: %ld\n", PTR_ERR(gpio_desc));
    return PTR_ERR(gpio_desc);
  }
  input_desc = gpio_desc;
  output_desc = devm_gpiod_get(&pdev->dev, "output", GPIOD_OUT_LOW);
  if (IS_ERR(output_desc)){
    return PTR_ERR(output_desc);
  }
  /* Get IRQ number from the descriptor */
  irq_number = gpiod_to_irq(gpio_desc);
  int int_irq_number = gpiod_to_irq(gpio_desc);
  if (int_irq_number < 0) {
    printk("Error!\nCannot get IRQ number\n");
    return irq_number;
  }
  /* Request the interrupt using device-managed resource */
  ret = devm_request_irq(&pdev->dev, irq_number, gpio_irq_handler,
                         IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "my_gpio_irq", NULL);
  if (ret != 0) {
    printk("Error!\nCannot request interrupt nr.: %d\n", irq_number);
    return ret;
  }

  return 0;
}

/**
 * @brief Platform driver remove function
 */
static void gpio_irq_remove(struct platform_device *pdev)
{
  /* No need to call free_irq() or gpiod_put() - automatically freed by devres */
}

/* Device tree match table */
static const struct of_device_id gpio_irq_of_match[] = {
  { .compatible = "custom,gpio-irq", },
  { }
};
MODULE_DEVICE_TABLE(of, gpio_irq_of_match);

/* Platform driver structure */
static struct platform_driver gpio_irq_driver = {
  .probe = gpio_irq_probe,
  .remove = gpio_irq_remove,
  .driver = {
    .name = "gpio-irq",
    .of_match_table = gpio_irq_of_match,
  },
};

/**
 * @brief This function is called when the module is loaded into the kernel
 */
static int __init gpio_irq_init(void)
{
  printk("gpio_irq: Registering platform driver...\n");
  return platform_driver_register(&gpio_irq_driver);
}

/**
 * @brief This function is called when the module is removed from the kernel
 */
static void __exit gpio_irq_exit(void)
{
  printk("gpio_irq: Unregistering platform driver...\n");
  platform_driver_unregister(&gpio_irq_driver);
}

module_init(gpio_irq_init);
module_exit(gpio_irq_exit);
