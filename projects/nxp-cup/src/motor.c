#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <math.h>

// I2C ADDRESS TESTING
#define I2C_ADDR 0x04 //AARDVARK
//#define I2C_ADDR 0x34

const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

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

int motor_drive(int regi, int power, int duration, int active_group[4]) 
{
    int recc_regi = 0x33; //(int)51 for motor encoder to drive motors.
    double abs_limits = 53.0;

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
    return ret;
}