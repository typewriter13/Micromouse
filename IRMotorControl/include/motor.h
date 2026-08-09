#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#define MOTOR_PIN_0A 15
#define MOTOR_PIN_0B 16
#define MOTOR_PIN_1A 17
#define MOTOR_PIN_1B 18

void pwm_left_motor_run(float duty_cycle);
void pwm_right_motor_run(float duty_cycle);
void pwm_left_motor_stop(void);
void pwm_right_motor_stop(void);
