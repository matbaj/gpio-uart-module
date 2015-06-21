#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/serio.h>

static DECLARE_WAIT_QUEUE_HEAD(wq);
# define SERIO_GPIOUART    0x40

static int uart_gpio_get(struct gpio_chip *chip, unsigned offset)
{
  //  return GPLR & GPIO_GPIO(offset);
  printk(KERN_INFO "[gpio-uart] %s Getting GPIO: %u \n", __func__,offset);
  return 0;
}

static void uart_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
//  unsigned char turn_on[] = { 0x1b };
//  unsigned char turn_off[] = { 0x1a };
  printk(KERN_INFO "[gpio-uart] %s Setting GPIO: %u value: %d\n", __func__,offset,value);
/*  if (value)
      //uart->f_op->write(uart, turn_on, 1, &uart->f_pos);
  else
      //uart->f_op->write(uart, turn_off, 1, &uart->f_pos); */
}

static int uart_direction_input(struct gpio_chip *chip, unsigned offset)
{
//    unsigned long flags;

 //   local_irq_save(flags);
 //   GPDR &= ~GPIO_GPIO(offset);
 //   local_irq_restore(flags);
    return 0; 
}

static int uart_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
 //   unsigned long flags;

 //   local_irq_save(flags);
 //   sa1100_gpio_set(chip, offset, value);
 //   GPDR |= GPIO_GPIO(offset);
 //   local_irq_restore(flags);
    return 0; 
}

static int uart_to_irq(struct gpio_chip *chip, unsigned offset)
{
 //   return offset < 11 ? (IRQ_GPIO0 + offset) : (IRQ_GPIO11 - 11 + offset);
      return 0;
}

static struct gpio_chip uart_gpio_chip = {
    .label          = "gpio",
    .direction_input    = uart_direction_input,
    .direction_output   = uart_direction_output,
    .set            = uart_gpio_set,
    .get            = uart_gpio_get,
    .to_irq         = uart_to_irq,
    .base           = 0, 
    .ngpio          = 2, 
};


static int gpio_connect(struct serio *serio, struct serio_driver *drv)
{
  int err;
  err = serio_open(serio, drv);
  if (err)
    return err;
  gpiochip_add(&uart_gpio_chip);

  return 0;
}

static void gpio_disconnect(struct serio *serio)
{
  serio_close(serio);
  gpiochip_remove(&uart_gpio_chip);
}



static struct serio_device_id gpio_serio_ids[] = {
  {
    .type = SERIO_RS232,
    .proto  = SERIO_GPIOUART,
    .id = SERIO_ANY,
    .extra  = SERIO_ANY,
  },
  { 0 }
};
MODULE_DEVICE_TABLE(serio, gpio_serio_ids);

static struct serio_driver gpio_drv = {
  .driver   = {
    .name = "gpio-uart",
  },
  .description  = "GPIOUART driver",
  .id_table = gpio_serio_ids,
  .connect  = gpio_connect,
  .disconnect = gpio_disconnect,
  .interrupt  = NULL,
};

module_serio_driver(gpio_drv);







MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mateusz Bajorski");
MODULE_DESCRIPTION("GPIO over uart");
