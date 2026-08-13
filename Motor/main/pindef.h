#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/mcpwm_prelude.h"
#include "esp_err.h"

#include <stdint.h>

#define MOTOR_PIN_0A 15
#define MOTOR_PIN_0B 16
#define MOTOR_PIN_1A 17
#define MOTOR_PIN_1B 18

void initialize_pwm_system(void);
void pwm_motor_forward(float duty_cycle);
void pwm_motor_backward(float duty_cycle);
