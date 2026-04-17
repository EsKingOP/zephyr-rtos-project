//RTOS PBL
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 1024
#define PRIORITY 5

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Queue for communication
K_MSGQ_DEFINE(parking_msgq, sizeof(int), 10, 4);

// Sensor Task (simulates entry/exit)
void sensor_task(void)
{
    int state = 1;

    while (1) {
        printk("Sensor: Car ENTERED\n");
        k_msgq_put(&parking_msgq, &state, K_NO_WAIT);

        k_sleep(K_SECONDS(5));

        state = 0;
        printk("Sensor: Car EXITED\n");
        k_msgq_put(&parking_msgq, &state, K_NO_WAIT);

        k_sleep(K_SECONDS(5));

        state = 1; // reset
    }
}

// Parking Task
void parking_task(void)
{
    int msg;

    while (1) {
        k_msgq_get(&parking_msgq, &msg, K_FOREVER);

        if (msg == 1) {
            gpio_pin_set_dt(&led, 1);
            printk("Slot: OCCUPIED\n");
        } else {
            gpio_pin_set_dt(&led, 0);
            printk("Slot: FREE\n");
        }
    }
}

K_THREAD_DEFINE(sensor_id, STACK_SIZE, sensor_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(parking_id, STACK_SIZE, parking_task, NULL, NULL, NULL, PRIORITY, 0, 0);

int main(void)
{
    if (!gpio_is_ready_dt(&led)) {
        printk("LED not ready\n");
        return 0;
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT);

    printk("Smart Parking FINAL System Started\n");

    return 0;
}