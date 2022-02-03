
/*
 * Copyright (C) 2022  University of Alberta
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
/**
 * @file    hardware_watchdog.c
 * @author  Josh Lazaruk
 * @date    2022-02--2
 */

#ifndef HARDWARE_WATCHDOG_C
#define HARDWARE_WATCHDOG_C

#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "HL_gio.h"

//Keeps watchdog timer pet. Timer is pet on both rising or falling edges
void obcHardwareWatchdog(void *pvParameters){

    int watchdog_time_s = 10;

    while(1){
        gioSetBit(gioPORTB, 4, 1);
        vTaskDelay(watchdog_time_s*1000);
        gioSetBit(gioPORTB, 4, 0);
        vTaskDelay(watchdog_time_s*1000);
    }
}

