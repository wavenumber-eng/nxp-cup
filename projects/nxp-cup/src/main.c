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

   int regi;
   sscanf(argv[2], "%d", &regi);

   //The neccesary reversal for same direction movement in motors 2 and 4 specifically.
   int move[4] = {0, 1, 0, -1};
   int ret = motor_drive(regi, drive_power, 0, move);
   if (ret) {
      printk("motor drive failed (%d)\n", ret);
   } else {
      printk("motor drive successful\n");
   }

   shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "Drive Power: %d\n", drive_power);
   shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "Register: %d\n", regi);

   return 0;
}

SHELL_CMD_REGISTER(monkey, NULL, "magic monkey", monkey_handler);
SHELL_CMD_REGISTER(motor_drive_power, NULL, "Returns hex number from decimal input for testing", motor_drive_power_handler);

int main(void)
{
	LOG_INF("starting...");

   int result;

   /*
   result = ble_init();
   if (result) {
      LOG_WRN("ble_init() failed");
   } else {
      LOG_INF("ble_init() successful");
   }
   */

   result = motor_init();
   if (result) {
      LOG_WRN("motor_init() failed");
   } else {
      LOG_INF("motor_init() successful");
   }

	while(1)
	{  
		k_sleep(K_MSEC(1000));
	}
}
