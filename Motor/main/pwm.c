#include "pindef.h"
#include "esp_log.h"

#define PWM_RES_HZ 1000000 // 1 MHz resolution (1us per tick)
#define PWM_FREQ_HZ 1000   // 1 kHz PWM frequency
#define PWM_PERIOD_TICKS (PWM_RES_HZ / PWM_FREQ_HZ) // 1000 ticks per period

static const char *TAG = "pwm_module";

// Static comparator handles to update duty cycle internally
static mcpwm_cmpr_handle_t cmpr_0a = NULL;
static mcpwm_cmpr_handle_t cmpr_0b = NULL;
static mcpwm_cmpr_handle_t cmpr_1a = NULL;
static mcpwm_cmpr_handle_t cmpr_1b = NULL;

void initialize_pwm_system(void)
{
    ESP_LOGI(TAG, "Initialize Timer");
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = PWM_RES_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = PWM_PERIOD_TICKS,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    ESP_LOGI(TAG, "Create Operators");
    mcpwm_oper_handle_t operator_0 = NULL;
    mcpwm_oper_handle_t operator_1 = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &operator_0));
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &operator_1));

    ESP_LOGI(TAG, "Connect Operators to Timer");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(operator_0, timer));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(operator_1, timer));

    ESP_LOGI(TAG, "Create Comparators");
    mcpwm_comparator_config_t compare_config = {
        .flags.update_cmp_on_tez = true,
    };
    // Operator 0 Comparators
    ESP_ERROR_CHECK(mcpwm_new_comparator(operator_0, &compare_config, &cmpr_0a));
    ESP_ERROR_CHECK(mcpwm_new_comparator(operator_0, &compare_config, &cmpr_0b));
    // Operator 1 Comparators
    ESP_ERROR_CHECK(mcpwm_new_comparator(operator_1, &compare_config, &cmpr_1a));
    ESP_ERROR_CHECK(mcpwm_new_comparator(operator_1, &compare_config, &cmpr_1b));

    // Initial duty cycle is 0%
    mcpwm_comparator_set_compare_value(cmpr_0a, 0);
    mcpwm_comparator_set_compare_value(cmpr_0b, 0);
    mcpwm_comparator_set_compare_value(cmpr_1a, 0);
    mcpwm_comparator_set_compare_value(cmpr_1b, 0);

    ESP_LOGI(TAG, "Create Generators");
    mcpwm_gen_handle_t gen_0a = NULL, gen_0b = NULL;
    mcpwm_gen_handle_t gen_1a = NULL, gen_1b = NULL;
    
    mcpwm_generator_config_t gen_config = {};
    
    gen_config.gen_gpio_num = MOTOR_PIN_0A;
    ESP_ERROR_CHECK(mcpwm_new_generator(operator_0, &gen_config, &gen_0a));
    gen_config.gen_gpio_num = MOTOR_PIN_0B;
    ESP_ERROR_CHECK(mcpwm_new_generator(operator_0, &gen_config, &gen_0b));
    
    gen_config.gen_gpio_num = MOTOR_PIN_1A;
    ESP_ERROR_CHECK(mcpwm_new_generator(operator_1, &gen_config, &gen_1a));
    gen_config.gen_gpio_num = MOTOR_PIN_1B;
    ESP_ERROR_CHECK(mcpwm_new_generator(operator_1, &gen_config, &gen_1b));

    ESP_LOGI(TAG, "Set Generator Actions (Active High)");
    // Generator 0A
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_0a, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_0a, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpr_0a, MCPWM_GEN_ACTION_LOW)));
    // Generator 0B
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_0b, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_0b, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpr_0b, MCPWM_GEN_ACTION_LOW)));
    // Generator 1A
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_1a, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_1a, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpr_1a, MCPWM_GEN_ACTION_LOW)));
    // Generator 1B
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_1b, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_1b, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpr_1b, MCPWM_GEN_ACTION_LOW)));

    ESP_LOGI(TAG, "Enable and Start Timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
}

// Helper to convert 0-100% duty cycle to timer ticks
static uint32_t calculate_compare_ticks(float duty_cycle) 
{
    if (duty_cycle > 100.0f) duty_cycle = 100.0f;
    if (duty_cycle < 0.0f) duty_cycle = 0.0f;
    return (uint32_t)((duty_cycle * PWM_PERIOD_TICKS) / 100.0f);
}

void pwm_motor_forward(float duty_cycle)
{
    uint32_t cmp_ticks = calculate_compare_ticks(duty_cycle);

    // Motor 0: A pulses, B is low (0 ticks)
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_0a, cmp_ticks));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_0b, 0));

    // Motor 1: A pulses, B is low (0 ticks)
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_1a, cmp_ticks));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_1b, 0));
}

void pwm_motor_backward(float duty_cycle)
{
    uint32_t cmp_ticks = calculate_compare_ticks(duty_cycle);

    // Motor 0: A is low (0 ticks), B pulses
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_0a, 0));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_0b, cmp_ticks));

    // Motor 1: A is low (0 ticks), B pulses
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_1a, 0));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_1b, cmp_ticks));
}
