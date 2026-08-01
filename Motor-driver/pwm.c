#include "pindefs.h"

// Global handles for the new API to update duty cycles later
mcpwm_cmpr_handle_t comparator_0 = NULL;
mcpwm_cmpr_handle_t comparator_1 = NULL;
mcpwm_gen_handle_t generator_0a = NULL;
mcpwm_gen_handle_t generator_0b = NULL;
mcpwm_gen_handle_t generator_1a = NULL;
mcpwm_gen_handle_t generator_1b = NULL;

#include "esp_err.h"

void pwm_gpio_configuration() {
    // 1. Initialize Timers
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000, 
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = 1000,     
    };
    mcpwm_timer_handle_t timer_0, timer_1;
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer_0));
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer_1));

    // 2. Initialize Operators
    mcpwm_operator_config_t oper_config = { .group_id = 0 };
    mcpwm_oper_handle_t oper_0, oper_1;
    ESP_ERROR_CHECK(mcpwm_new_operator(&oper_config, &oper_0));
    ESP_ERROR_CHECK(mcpwm_new_operator(&oper_config, &oper_1));

    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_0, timer_0));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_1, timer_1));

    // 3. Initialize Comparators
    mcpwm_comparator_config_t cmpr_config = { .flags.update_cmp_on_tez = true };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_0, &cmpr_config, &comparator_0));
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_1, &cmpr_config, &comparator_1));

    // 4. Initialize Generators (Bind to GPIOs)
    mcpwm_generator_config_t gen_config_0a = { .gen_gpio_num = MOTOR_PIN_0A };
    mcpwm_generator_config_t gen_config_0b = { .gen_gpio_num = MOTOR_PIN_0B };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_0, &gen_config_0a, &generator_0a));
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_0, &gen_config_0b, &generator_0b));

    mcpwm_generator_config_t gen_config_1a = { .gen_gpio_num = MOTOR_PIN_1A };
    mcpwm_generator_config_t gen_config_1b = { .gen_gpio_num = MOTOR_PIN_1B };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_1, &gen_config_1a, &generator_1a));
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_1, &gen_config_1b, &generator_1b));

    // 5. Define PWM Event Actions
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator_0a, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator_0a, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_0, MCPWM_GEN_ACTION_LOW)));

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator_0b, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator_0b, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_0, MCPWM_GEN_ACTION_LOW)));

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator_1a, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator_1a, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_1, MCPWM_GEN_ACTION_LOW)));

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator_1b, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator_1b, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_1, MCPWM_GEN_ACTION_LOW)));

    // 6. Enable and Start Timers
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer_0));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer_0, MCPWM_TIMER_START_NO_STOP));
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer_1));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer_1, MCPWM_TIMER_START_NO_STOP));
}

void pwm_motor_forward(float duty_cycle) {
    // Convert percentage (0-100) to timer ticks (0-1000)
    uint32_t compare_val = (uint32_t)(duty_cycle * 10.0f);

    // Set PWM Duty Cycles
    mcpwm_comparator_set_compare_value(comparator_0, compare_val);
    mcpwm_comparator_set_compare_value(comparator_1, compare_val);

    // Force 'B' pins to standard LOW (0) and unforce (-1) 'A' pins to let PWM run
    mcpwm_generator_set_force_level(generator_0b, 0, true);
    mcpwm_generator_set_force_level(generator_0a, -1, true);

    mcpwm_generator_set_force_level(generator_1b, 0, true);
    mcpwm_generator_set_force_level(generator_1a, -1, true);
}

void pwm_motor_backward(float duty_cycle) {
    // Convert percentage (0-100) to timer ticks (0-1000)
    uint32_t compare_val = (uint32_t)(duty_cycle * 10.0f);

    // Set PWM Duty Cycles
    mcpwm_comparator_set_compare_value(comparator_0, compare_val);
    mcpwm_comparator_set_compare_value(comparator_1, compare_val);

    // Force 'A' pins to standard LOW (0) and unforce (-1) 'B' pins to let PWM run
    mcpwm_generator_set_force_level(generator_0a, 0, true);
    mcpwm_generator_set_force_level(generator_0b, -1, true);

    mcpwm_generator_set_force_level(generator_1a, 0, true);
    mcpwm_generator_set_force_level(generator_1b, -1, true);
}