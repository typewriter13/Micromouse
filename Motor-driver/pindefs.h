/*
 pindefs.h*
 Created on: 14-Jul-2026
 Author: shrey*/

#ifndef MAIN_PINDEFSH
#define MAIN_PINDEFSH

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Replaced deprecated libraries with the new v5.0+ driver
#include "driver/mcpwm_prelude.h"

#define MOTOR_PIN_0A 4
#define MOTOR_PIN_0B 5

#define MOTOR_PIN_1A 6
#define MOTOR_PIN_1B 7

#endif /* MAIN_PINDEFSH */