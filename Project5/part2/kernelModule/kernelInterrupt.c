#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("NN");
MODULE_DESCRIPTION("A simple LKM for gpio interrupt using pure descriptor API");


/** Variable contains IRQ number */
static unsigned int irq_number;
static struct gpio_desc *c1_desc;
static struct gpio_desc *c2_desc;
static int pos = 0;
#define DEVICE_NAME "encoder"
static int majorNumber;
module_param(pos, int, 0444);


/**
 * @brief Interrupt service routine is called, when interrupt is triggered
 */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    if (gpiod_get_value(c1_desc) == gpiod_get_value(c2_desc))
        pos++;
    else
        pos--;
  //printk("counter = %d\n", pos);
  return IRQ_HANDLED;
}

/**
 * @brief Platform driver probe function
 */
static int gpio_irq_probe(struct platform_device *pdev)
{
  int ret;
  /* Get GPIO descriptor using device-managed resource */

  c1_desc = devm_gpiod_get(&pdev->dev, "c1", GPIOD_IN);
  if (IS_ERR(c1_desc)) {
    return PTR_ERR(c1_desc);
  }

  c2_desc = devm_gpiod_get(&pdev->dev, "c2", GPIOD_IN);
  if (IS_ERR(c2_desc))
    return PTR_ERR(c2_desc);

  /* Get IRQ number from the descriptor */
  irq_number = gpiod_to_irq(c1_desc);
  int int_irq_number = gpiod_to_irq(c1_desc);
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

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
  int value = READ_ONCE(pos);

  if (len < sizeof(value))
    return -EINVAL;
  if (copy_to_user(buf, &value, sizeof(value)))
    return -EFAULT;
  return sizeof(value);
}

static const struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = dev_read,
};

/**
 * @brief This function is called when the module is loaded into the kernel
 */
static int __init gpio_irq_init(void)
{
int ret;

  printk("gpio_irq: Registering platform driver...\n");
  majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
  if (majorNumber < 0)
    return majorNumber;

  ret = platform_driver_register(&gpio_irq_driver);
  if (ret)
    unregister_chrdev(majorNumber, DEVICE_NAME);
  return ret;
}

/**
 * @brief This function is called when the module is removed from the kernel
 */
static void __exit gpio_irq_exit(void)
{
  printk("gpio_irq: Unregistering platform driver...\n");
  platform_driver_unregister(&gpio_irq_driver);
  unregister_chrdev(majorNumber, DEVICE_NAME);
}

module_init(gpio_irq_init);
module_exit(gpio_irq_exit);
