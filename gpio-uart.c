#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/serio.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/sched.h>
#define SERIO_GPIOUART    0x40
#define GPIO_COUNT 8
#define GPIO_STATE_IDLE 0
#define GPIO_STATE_READ 1

static DECLARE_WAIT_QUEUE_HEAD(wq);


static unsigned int uart_gpio_counter;

struct gpio_data {
	int state;
	unsigned int gpio_state[GPIO_COUNT];
	unsigned char buf;
	struct serio *serio;
	struct gpio_chip gpio_chip;
};

static int uart_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct gpio_data *gpio_d;
	unsigned int cmd;
	struct serio *serio;

	gpio_d = chip->irqdomain->host_data;
	serio = gpio_d->serio;
	cmd = offset<<1; /* shift one to left */

	serio_write(serio, cmd);
	pr_info("[gpio-uart] %s Getting GPIO: %u  cmd: %u\n",
		__func__, offset, cmd);
	gpio_d->state = GPIO_STATE_READ;
	wait_event_interruptible_timeout(wq, gpio_d->state == GPIO_STATE_IDLE,
		msecs_to_jiffies(2000));

	if (gpio_d->state != GPIO_STATE_IDLE)  {
		pr_err("[gpio-uart] %s GPIO state is not idle\n",
			__func__);
		return 0;
	}  else  {
		return (int)gpio_d->buf;
	}
}

static void uart_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct gpio_data *gpio_d;
	struct serio *serio;
	unsigned int cmd;

	pr_info("[gpio-uart] %s Setting GPIO: %u value: %d\n",
		__func__, offset, value);

	gpio_d = chip->irqdomain->host_data;
	serio = gpio_d->serio;

	cmd = offset<<1; /* Shift one place to left to leave place for value */
	cmd |= 0x10; /* Write command */
	cmd |= value; /* value */
	pr_info("[gpio-uart] %s Setting GPIO: %u value: %d cmd: 0x%x\n",
		__func__, offset, value, cmd);
	serio_write(serio, cmd);
}

static int uart_direction_input(struct gpio_chip *chip, unsigned offset)
{
	/* TODO */
	return 0;
}

static int uart_direction_output(struct gpio_chip *chip, unsigned offset,
	int value)
{
	/* TODO */
	return 0;
}

static int uart_to_irq(struct gpio_chip *chip, unsigned offset)
{
	/* TODO */
	return 0;
}
static irqreturn_t uart_interrupt(struct serio *serio, unsigned char data,
	unsigned int flags)
{
	struct gpio_data *gpio_d = serio_get_drvdata(serio);

	switch (gpio_d->state) {
	case GPIO_STATE_READ:
		gpio_d->buf = data;
		gpio_d->state = GPIO_STATE_IDLE;
		wake_up_interruptible(&wq);
		break;
	case GPIO_STATE_IDLE:
		/* It would be cool to implement gpio interrupt */
		break;
	}
	return IRQ_HANDLED;

}


static int gpio_connect(struct serio *serio, struct serio_driver *drv)
{
	int err;
	struct gpio_data *gpio_d;
	struct irq_domain *irqdomain;
	struct gpio_chip gpio_chip = {
		.label          = "gpio",
		.direction_input    = uart_direction_input,
		.direction_output   = uart_direction_output,
		.set            = uart_gpio_set,
		.get            = uart_gpio_get,
		.to_irq         = uart_to_irq,
		.ngpio          = uart_gpio_counter*GPIO_COUNT+GPIO_COUNT,
		.base           = uart_gpio_counter*GPIO_COUNT,
		.owner          = THIS_MODULE,
	};
	gpio_d = kzalloc(sizeof(struct gpio_data), GFP_KERNEL);
	if (!gpio_d)
		return -ENOMEM;

	gpio_d->state = GPIO_STATE_IDLE;
	gpio_d->serio = serio;
	gpio_d->gpio_chip = gpio_chip;

	irqdomain = kzalloc(sizeof(struct irq_domain), GFP_KERNEL);
	if (!irqdomain)
		return -ENOMEM;
	irqdomain->host_data = gpio_d;
	gpio_d->gpio_chip.irqdomain = irqdomain;

	serio_set_drvdata(serio, gpio_d);
	err = serio_open(serio, drv);
	if (err)
		return err;
	gpiochip_add(&(gpio_d->gpio_chip));

	uart_gpio_counter++;
	return 0;
}

static void gpio_disconnect(struct serio *serio)
{
	struct gpio_data *gpio_d = serio_get_drvdata(serio);
	struct gpio_chip *gpio_chip = &gpio_d->gpio_chip;

	kzfree(gpio_chip->irqdomain);
	gpio_d->gpio_chip.irqdomain = NULL;

	gpiochip_remove(gpio_chip);
	kzfree(gpio_d);
	serio_close(serio);
	uart_gpio_counter--;
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
	.interrupt  = uart_interrupt,
};

module_serio_driver(gpio_drv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mateusz Bajorski");
MODULE_DESCRIPTION("GPIO over uart");
