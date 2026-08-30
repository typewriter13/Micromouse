#include "../include/motor.h"

void initialize_pwm_gpio_pins()
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_PIN_0A);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, MOTOR_PIN_0B);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, MOTOR_PIN_1A);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, MOTOR_PIN_1B);
}

void pwm_gpio_configuration()
{
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;
    pwm_config.cmpr_a = 0;
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);
}

void pwm_left_motor_run(float duty_cycle)
{
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
}

void pwm_right_motor_run(float duty_cycle)
{
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
}

void pwm_left_motor_reverse(float duty_cycle)
{

    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
}

void pwm_right_motor_reverse(float duty_cycle)
{
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
}

void pwm_motors_brake(void)
{
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A);
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B);
}

void turn_left(void)
{
    pwm_left_motor_run(min_duty_cycle);
    pwm_right_motor_run(max_duty_cycle);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    pwm_motors_brake();
}

void turn_right(void)
{
    pwm_left_motor_run(max_duty_cycle);
    pwm_right_motor_run(min_duty_cycle);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    pwm_motors_brake();
}

void turn_around(void)
{
    pwm_right_motor_run(30.0);
    pwm_left_motor_reverse(30.0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    pwm_motors_brake();
}
