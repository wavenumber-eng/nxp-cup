/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

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


SHELL_CMD_REGISTER(monkey, NULL, "magic monkey", monkey_handler);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

extern void ble_init(void);

int main(void) {
    LOG_INF("Hello world");

    // Initialize BLE
    ble_init();

    while (1) {
        k_sleep(K_MSEC(1000));
    }

    return 0;
}
