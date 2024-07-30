/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <stdio.h>
#include <stdlib.h>

#include "ble.h"
#include "motor.h"

LOG_MODULE_REGISTER(main);

static int monkey_handler(const struct shell *shell, 
                      size_t argc,
                      char **argv)
{
   ARG_UNUSED(argc);
   ARG_UNUSED(argv);

   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"\r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"                 ██████████████████████████            \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"               ██▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒██          \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"               ██▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒██        \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"             ██▒▒▒▒░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░██        \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"         ██████▒▒░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░██      \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"       ██░░░░░░▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░██████  \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"       ██░░░░░░▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░██░░░░██\r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"       ██░░░░░░▒▒░░░░░░░░░░██░░░░░░░░██░░░░░░░░██░░░░██\r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"         ████░░▒▒░░░░░░░░░░██░░░░░░░░██░░░░░░░░██████  \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"             ██▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░██      \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"     ████      ██▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░██        \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"   ██    ██      ██▒▒░░░░░░░░░░░░░░░░░░░░░░██          \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"   ██  ██      ██▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒██        \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"   ██          ██▒▒▒▒▒▒▒▒░░░░░░░░░░░░▒▒▒▒▒▒▒▒██        \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"     ████    ██▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒██      \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"         ██████▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒██      \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"             ██▒▒▒▒██▒▒░░░░░░░░░░░░░░░░▒▒██▒▒▒▒██      \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"               ██████▒▒▒▒░░░░░░░░░░░░▒▒▒▒██████        \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"                   ██▒▒▒▒▒▒████████▒▒▒▒▒▒██            \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"                   ██░░░░██        ██░░░░██            \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"                   ██████            ██████            \r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_YELLOW,"\r\n");
   shell_fprintf(shell,SHELL_VT100_COLOR_CYAN,"                          I'm Super\r\n");
      
   return 0;
}

static int motor_drive_power_handler(const struct shell *shell,
                                     size_t argc,
                                     char **argv)
{
   ARG_UNUSED(argc);

   int drive_power;
   sscanf(argv[1], "%d", &drive_power);

   int wait_time;
   sscanf(argv[2], "%d", &wait_time);

   int regi;
   sscanf(argv[3], "%d", &regi);

   //The neccesary reversal for same direction movement in motors 2 and 4 specifically.
   int move[4] = {0, 1, 0, -1};
   int ret = motor_drive(regi, drive_power, wait_time, move);
   if (ret) {
      printk("motor drive failed (%d)\n", ret);
   } else {
      printk("motor drive successful\n");
   }

   shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "Drive Power: %d\n", drive_power);
   shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "Duration: %d\n", wait_time);
   shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "Register: %d\n", regi);

   return 0;
}

static int motor_interp_power_handler(const struct shell *shell,
                                     size_t argc,
                                     char **argv)
{
   ARG_UNUSED(argc);

   int power_srt;
   sscanf(argv[1], "%d", &power_srt);

   int power_end;
   sscanf(argv[2], "%d", &power_end);

   int step;
   sscanf(argv[3], "%d", &step);

   int regi;
   sscanf(argv[4], "%d", &regi);

   int move[4] = {0, 1, 0, -1};
   int ret = motor_interp(regi, power_srt, power_end, step, move);
   if (ret) {
      printk("motor interp failed (%d)\n", ret);
   } else {
      printk("motor interp successful\n");
   }

   return 0;
}

static int motor_steer_angle_handler(const struct shell *shell,
                                     size_t argc,
                                     char **argv)
{
   ARG_UNUSED(argc);

   int angle_srt;
   sscanf(argv[1], "%d", &angle_srt);

   int angle_end;
   sscanf(argv[2], "%d", &angle_end);


   int ret = motor_servo(angle_srt, angle_end);
   if (ret) {
      printk("motor steer failed (%d)\n", ret);
   } else {
      printk("motor steer successful\n");
   }
}


SHELL_CMD_REGISTER(monkey, NULL, "magic monkey", monkey_handler);
SHELL_CMD_REGISTER(motor_drive_power, NULL, "Drive the motor for a duration, duration 0 is ignored", motor_drive_power_handler);
SHELL_CMD_REGISTER(motor_interp_power, NULL, "Move from one speed to another", motor_interp_power_handler);
SHELL_CMD_REGISTER(motor_steer_angle, NULL, "Set steering angle", motor_steer_angle_handler);

int main(void)
{
	printk("starting...\n\n");

   int result;

   // Checks and sets up the BLE service.
   result = ble_init();
   if (result) {
      printk("ble_init() failed\n\n");
   } else {
      printk("ble_init() successful\n\n");
   }
   

  // Checks if the I2C motor encoder device is ready.
   result = motor_init();
   if (result) {
      printk("motor_init() failed\n\n");
   } else {
      printk("motor_init() successful\n\n");
   }

	while(1)
	{  
		k_sleep(K_MSEC(1000));
	}
}
