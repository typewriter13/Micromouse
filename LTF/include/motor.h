#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#define MOTOR_PIN_0A 5
#define MOTOR_PIN_0B 4
#define MOTOR_PIN_1A 6
#define MOTOR_PIN_1B 7
#define UTURN_TIME_MS   200
#define Max_duty_cycle 60.0
#define Min_duty_cycle 10.0
// #define slow_duty_cycle 20.0
// #define fast_duty_cycle 55.0

void initialize_pwm_gpio_pins();
void pwm_gpio_configuration();
void pwm_left_motor_run(float duty_cycle);
void pwm_right_motor_run(float duty_cycle);
void pwm_left_motor_reverse(float duty_cycle);
void pwm_right_motor_reverse(float duty_cycle);
void turn_around(void);
void turn_right(void);
void turn_left(void);
void pwm_motors_brake(void);
