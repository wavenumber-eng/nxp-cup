#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <math.h>
#include <zephyr/kernel.h>

// I2C ADDRESS TESTING
//#define I2C_ADDR 0x04 //AARDVARK
#define I2C_ADDR 0x34 //MOTOR ENCODER

#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY K_PRIO_PREEMPT(1)

const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

K_THREAD_STACK_DEFINE(thread_stack, THREAD_STACK_SIZE);
static struct k_thread stop_thread_data;

int motor_init(void)
{
    if (!device_is_ready(i2c_dev)) {
        printk("I2C: Device is not ready.\n");
        return -ENODEV;
    } else {
        printk("I2C: Device is ready.\n");
    }
    return 0;
}

// Thread function to perform the stop I2C operation
void motor_stop_thread_function(void *i2c_dev_ptr, void *regi_ptr, void *duration_ptr)
{
    const struct device *i2c_dev = (const struct device *)i2c_dev_ptr;
    int *regi_use = (uint8_t *)regi_ptr;
    int duration = *(int *)duration_ptr;

    uint8_t stop_data[] = {regi_use, 0x00, 0x00, 0x00, 0x00};
    k_sleep(K_MSEC(duration * 1000)); // Sleep for the specified duration
    i2c_write(i2c_dev, stop_data, 5, I2C_ADDR); // Write the stop data
}

int motor_drive(int regi, int power, int duration, int active_group[4]) 
{
    int recc_regi = 0x33; //(int)51 for motor encoder to drive motors.
    double abs_limits = 53.0; // Found through experiment.

    int regi_use;
    if(regi == 0) {
        regi_use = recc_regi;
        printk("Using default register 0x%X\n", regi_use);
    } else {
        regi_use = regi;
        printk("Using custom register: 0x%X\n", regi_use);
    }

    // Int power comes in as a number -100 < power < 100.
    // Needs to be constrained to an integer between 53 and -53 using the abs_limits.
    int scaled_power = (int)round((double)power * (abs_limits / 100.0));

    // active_group[4] comes in as an array of ints {0, 1, 0 -1} for instance.
    uint8_t data[] = {regi_use, (uint8_t)(active_group[0] * scaled_power),
                            (uint8_t)(active_group[1] * scaled_power),
                            (uint8_t)(active_group[2] * scaled_power),
                            (uint8_t)(active_group[3] * scaled_power)};
    
    printk("Scaled Power: %d\n", scaled_power);
    // Perform the I2C write operation
    int ret = i2c_write(i2c_dev, data, ARRAY_SIZE(data), I2C_ADDR);
    if (duration > 0) {
        k_tid_t thread_id = k_thread_create(&stop_thread_data, thread_stack, THREAD_STACK_SIZE,
                        motor_stop_thread_function, (void *)i2c_dev, (void *)regi_use, (void *)&duration,
                        THREAD_PRIORITY, 0, K_NO_WAIT);
        if (thread_id == NULL) {
            printk("Failed to create thread\n");
        } else {
            printk("Thread created successfully\n");
        }
    }

    return ret;
}

int motor_interp(int regi, int start_pwr, int end_pwr, int step, int active_group[4])
{
    int recc_regi = 0x33; //(int)51 for motor encoder to drive motors.
    double abs_limits = 53.0; // Found through experiment.

    int regi_use;
    if(regi == 0) {
        regi_use = recc_regi;
        printk("Using default register 0x%X\n", regi_use);
    } else {
        regi_use = regi;
        printk("Using custom register: 0x%X\n", regi_use);
    }

    // Int power comes in as a number -100 < power < 100.
    // Needs to be constrained to an integer between 53 and -53 using the abs_limits.
    int scaled_srt_power = (int)round((double)start_pwr * (abs_limits / 100.0));
    int scaled_end_power = (int)round((double)end_pwr * (abs_limits / 100.0));

    if(scaled_srt_power < scaled_end_power)
    {
        for(int drive_power = scaled_srt_power; drive_power <= scaled_end_power; drive_power++)
        {
            uint8_t data[] = {regi_use, (uint8_t)(active_group[0] * drive_power),
                            (uint8_t)(active_group[1] * drive_power),
                            (uint8_t)(active_group[2] * drive_power),
                            (uint8_t)(active_group[3] * drive_power)};
            k_sleep(K_MSEC(step));
            int ret = i2c_write(i2c_dev, data, ARRAY_SIZE(data), I2C_ADDR);              
        }
    } else {
        for(int drive_power = scaled_srt_power; drive_power >= scaled_end_power; drive_power--)
        {
            uint8_t data[] = {regi_use, (uint8_t)(active_group[0] * drive_power),
                            (uint8_t)(active_group[1] * drive_power),
                            (uint8_t)(active_group[2] * drive_power),
                            (uint8_t)(active_group[3] * drive_power)};
            k_sleep(K_MSEC(step));
            int ret = i2c_write(i2c_dev, data, ARRAY_SIZE(data), I2C_ADDR);              
        }
    }

}