#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <math.h>
#include <zephyr/kernel.h>

#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

#include <zephyr/sys/atomic.h>

// I2C ADDRESS TESTING
//#define I2C_ADDR 0x04 //AARDVARK
#define I2C_ADDR 0x34 //MOTOR ENCODER

// Threading for Motor Interpolation
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY K_PRIO_PREEMPT(1)

// PWM defines
#define PWM_NODE DT_NODELABEL(pwm0)
#define PWM_CHANNEL 0                   /* Channel 0 corresponds to PWM_OUT0 */
#define PWM_PERIOD_USEC 2.857 * 1000 * 1000    // Mimic 350Hz frequency
#define PWM_POLARITY_NORMAL 0           // Normal polarity for the PWM signal
#define SLEW 1                         // Slew rate to move Servo

#define STEER_CLAMP 90

// Statics for storing the state POWER and ANGLE of the motors in the car
static int state_drive = 0;
static int state_steer = 0; 

// Atomics for storing target data (The target states of the system) (POWER and ANGLE)
atomic_t state_drive_tgt = ATOMIC_INIT(0);
atomic_t state_steer_tgt = ATOMIC_INIT(0);

const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

K_THREAD_STACK_DEFINE(thread_stack, THREAD_STACK_SIZE);
static struct k_thread stop_thread_data;

// Initialize all the motors before use
int motor_init(void)
{
    if (!device_is_ready(i2c_dev)) {
        printk("motor_init: Device is not ready.\n");
        return -ENODEV;
    } else {
        printk("motor_init: Device is ready.\n");
    }
    return 0;

    // Sweep the steering motor to catch current state and set it to 0
    const struct device *pwm_dev;
    pwm_dev = DEVICE_DT_GET(PWM_NODE);
    int ret;
    for(int sweep = 1000000; sweep <= 2000000; sweep+= 1000) {
        ret = pwm_set(pwm_dev, PWM_CHANNEL, PWM_PERIOD_USEC, sweep, NULL);
        k_sleep(K_MSEC(SLEW));
    }
    // Sweep with motor locked back to home
    for(int sweep = 2000000; sweep >= 1500000; sweep-= 1000) {
        ret = pwm_set(pwm_dev, PWM_CHANNEL, PWM_PERIOD_USEC, sweep, NULL);
        k_sleep(K_MSEC(SLEW));
    }
    if(ret) {
        printk("motor_init: Error in Calibration.\n");
        return ret;
    } else {
        printk("motor_init: Steering ready.\n");
        return 0;
    }
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

// Function to manually set the power for the drive motors
int motor_drive(int regi, int power, int duration, int active_group[4]) 
{
    int recc_regi = 0x33; //(int)51 for motor encoder to drive motors.
    double abs_limits = 53.0; // Found through experiment.

    int regi_use;
    if(regi == 0) {
        regi_use = recc_regi;
        printk("motor_drive: Using default register 0x%X\n", regi_use);
    } else {
        regi_use = regi;
        printk("motor_drive: Using custom register: 0x%X\n", regi_use);
    }

    // Int power comes in as a number -100 < power < 100.
    // Needs to be constrained to an integer between 53 and -53 using the abs_limits.
    int scaled_power = (int)round((double)power * (abs_limits / 100.0));

    // active_group[4] comes in as an array of ints {0, 1, 0 -1} for instance.
    uint8_t data[] = {regi_use, (uint8_t)(active_group[0] * scaled_power),
                            (uint8_t)(active_group[1] * scaled_power),
                            (uint8_t)(active_group[2] * scaled_power),
                            (uint8_t)(active_group[3] * scaled_power)};
    
    printk("motor_drive: Scaled Power: %d\n", scaled_power);
    // Perform the I2C write operation
    int ret = i2c_write(i2c_dev, data, ARRAY_SIZE(data), I2C_ADDR);
    if (duration > 0) {
        k_tid_t thread_id = k_thread_create(&stop_thread_data, thread_stack, THREAD_STACK_SIZE,
                        motor_stop_thread_function, (void *)i2c_dev, (void *)regi_use, (void *)&duration,
                        THREAD_PRIORITY, 0, K_NO_WAIT);
        if (thread_id == NULL) {
            printk("motor_drive: Failed to create thread\n");
        } else {
            printk("motor_drive: Thread created successfully\n");
        }
    }

    return ret;
}

// Function to manually move the drive motors to a speed raming up from a start power to end power
int motor_interp(int regi, int start_pwr, int end_pwr, int step, int active_group[4])
{
    int recc_regi = 0x33; //(int)51 for motor encoder to drive motors.
    double abs_limits = 53.0; // Found through experiment.

    int regi_use;
    if(regi == 0) {
        regi_use = recc_regi;
        printk("motor_interp: Using default register 0x%X\n", regi_use);
    } else {
        regi_use = regi;
        printk("motor_interp: Using custom register: 0x%X\n", regi_use);
    }

    // Int power comes in as a number -100 < power < 100.
    // Needs to be constrained to an integer between 53 and -53 using the abs_limits.
    int scaled_srt_power = (int)round((double)start_pwr * (abs_limits / 100.0));
    int scaled_end_power = (int)round((double)end_pwr * (abs_limits / 100.0));

    int ret = 0;
    if(scaled_srt_power < scaled_end_power)
    {
        for(int drive_power = scaled_srt_power; drive_power <= scaled_end_power; drive_power++)
        {
            uint8_t data[] = {regi_use, (uint8_t)(active_group[0] * drive_power),
                            (uint8_t)(active_group[1] * drive_power),
                            (uint8_t)(active_group[2] * drive_power),
                            (uint8_t)(active_group[3] * drive_power)};
            int ret = i2c_write(i2c_dev, data, ARRAY_SIZE(data), I2C_ADDR);
            k_sleep(K_MSEC(step));
        }
    } else {
        for(int drive_power = scaled_srt_power; drive_power >= scaled_end_power; drive_power--)
        {
            uint8_t data[] = {regi_use, (uint8_t)(active_group[0] * drive_power),
                            (uint8_t)(active_group[1] * drive_power),
                            (uint8_t)(active_group[2] * drive_power),
                            (uint8_t)(active_group[3] * drive_power)};
            int ret = i2c_write(i2c_dev, data, ARRAY_SIZE(data), I2C_ADDR);
            k_sleep(K_MSEC(step));
        }
    }

    return ret;
}

// Function to manually move the steering servo to a desired end angle while catching a range of angles inbetween the start and end
int motor_interp_servo(int angle_srt, int angle_end)
{
    const struct device *pwm_dev;
    int pulse_width_srt;
    int pulse_width_end;
    int ret;

    // Get the PWM device from the device tree
    pwm_dev = DEVICE_DT_GET(PWM_NODE);
    if (!device_is_ready(pwm_dev)) {
        printk("motor_interp_servo: PWM device %s is not ready", pwm_dev->name);
        return -ENODEV; // Return an error code if the device is not ready
    }

    // Calculate pulse width based on the angle
    pulse_width_srt = 1000*(int)round(1000 + (angle_srt + STEER_CLAMP) * (1000 / (STEER_CLAMP * 2)));
    pulse_width_end = 1000*(int)round(1000 + (angle_end + STEER_CLAMP) * (1000 / (STEER_CLAMP * 2)));

    if(pulse_width_srt < pulse_width_end)
    {
        for(int pulse_width = pulse_width_srt; pulse_width <= pulse_width_end; pulse_width+= 1000)
        {
            k_sleep(K_MSEC(SLEW));
            int ret = pwm_set(pwm_dev, PWM_CHANNEL, PWM_PERIOD_USEC, pulse_width, NULL);
        }
    } else {
        for(int pulse_width = pulse_width_srt; pulse_width >= pulse_width_end; pulse_width-= 1000)
        {
            k_sleep(K_MSEC(SLEW));
            int ret = pwm_set(pwm_dev, PWM_CHANNEL, PWM_PERIOD_USEC, pulse_width, NULL);
        }
    }
    if (ret) {
        printk("motor_interp_servo: Failed to set PWM signal. Error %d", ret);
        return ret; // Return the error code if setting PWM fails
    }

    printk("motor_interp_servo: PWM pulse width set to %d ns for angle %d degrees\n", pulse_width_end, angle_end);
    return 0; // Return 0 for success
}

// Function to manually set the steering servo to an angle
int motor_servo(int angle)
{
    const struct device *pwm_dev;
    int pulse_width;
    int ret;

    // Get the PWM device from the device tree
    pwm_dev = DEVICE_DT_GET(PWM_NODE);
    if (!device_is_ready(pwm_dev)) {
        printk("motor_servo: PWM device %s is not ready", pwm_dev->name);
        return -ENODEV; // Return an error code if the device is not ready
    }

    // Calculate pulse width based on the angle
    pulse_width = 1000*(int)round(1000 + (angle + STEER_CLAMP) * (1000 / (STEER_CLAMP * 2)));
    ret = pwm_set(pwm_dev, PWM_CHANNEL, PWM_PERIOD_USEC, pulse_width, NULL);

    if (ret) {
        printk("motor_servo: Failed to set PWM signal. Error %d", ret);
        return ret; // Return the error code if setting PWM fails
    }

    printk("motor_servo: PWM pulse width set to %d ns for angle %d degrees\n", pulse_width, angle);
    return 0; // Return 0 for success
}

// Function to look at deltas between both steering and drive variable to slew them towards their command variables
int motor_task()
{
    int ret;
    static int move[4] = {0, 1, 0, -1};

    if(atomic_get(&state_steer_tgt) > state_steer)
    {
        state_steer++;
    } else if(atomic_get(&state_steer_tgt) < state_steer) {
        state_steer--;
    } else {
        // Keep the steer state the same.
    }
    if(atomic_get(&state_steer_tgt) != state_steer)
    {
        ret = motor_servo(state_steer);
        printk("motor_task: steer tgt: %d\n", atomic_get(&state_steer_tgt));
        printk("motor_task: steer: %d\n", state_steer);
    }

    if(atomic_get(&state_drive_tgt) > state_drive)
    {
        state_drive++;
    } else if(atomic_get(&state_drive_tgt) > state_drive) {
        state_drive--;
    } else {
        // Keep the Drive State the same.
    }
    if(atomic_get(&state_drive_tgt) != state_drive)
    {
        ret = motor_drive(0x33, state_drive, 0x00, move);
    }

    return ret;
}

int motor_set_state_steer_tgt(int tgt)
{
    atomic_set(&state_steer_tgt, tgt);
}

int motor_set_state_drive_tgt(int tgt)
{
    atomic_set(&state_drive_tgt, tgt);
}