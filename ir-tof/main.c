#include <stdio.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_NUM         I2C_NUM_0
#define I2C_MASTER_SDA_IO      41
#define I2C_MASTER_SCL_IO      42
#define I2C_MASTER_FREQ_HZ     100000

#define VL53L0X_ADDR           0x29  // Default factory I2C address
#define SYSRANGE_START_REG     0x00
#define RESULT_RANGE_REG       0x1E

#define LOWER_THRESHOLD        20
#define UPPER_THRESHOLD        2000

#define IR_ADC_CHAN            ADC_CHANNEL_7     // GPIO4 maps to ADC1_7
#define ADC_BITWIDTH           ADC_BITWIDTH_12   // 12-bit (0-4095)
#define ADC_ATTEN              ADC_ATTEN_DB_11   // Safe up to ~3.1V/3.3V

static const char *TAG = "DUAL_SENSOR";

adc_oneshot_unit_handle_t adc1_handle;

// I2C Comm Drivers

esp_err_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_write_to_device(I2C_MASTER_NUM, dev_addr, write_buf, 2, 1000 / portTICK_PERIOD_MS);
}

esp_err_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(I2C_MASTER_NUM, dev_addr, &reg, 1, data, len, 1000 / portTICK_PERIOD_MS);
}

void tof_i2c_init(void) {
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0));
    ESP_LOGI(TAG, "I2C initialized for ToF sensor.");
}

// Analog IR Initialization
void ir_adc_init(void) {
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, IR_ADC_CHAN, &config));
    ESP_LOGI(TAG, "Analog IR setup initialized on GPIO4.");
}

void app_main() {
    // 1. Initialize Peripherals
    tof_i2c_init();
    ir_adc_init();

    // Verification check to make sure the single ToF sensor is online
    uint8_t model_id;
    if (i2c_read_reg(VL53L0X_ADDR, 0xC0, &model_id, 1) != ESP_OK) {
        ESP_LOGE(TAG, "ToF Sensor failed to respond at address 0x29! Check I2C wiring.");
    } else {
        ESP_LOGI(TAG, "ToF Sensor detected successfully.");
    }

    int ir_raw_value = 0;
    uint16_t tof_distance_mm = 0;
    uint8_t dist_data[2];

    while (1) {
        // --- 1. Read Analog IR Proximity Sensor ---
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_ADC_CHAN, &ir_raw_value));
        
        // --- 2. Read Single ToF Distance Sensor ---
        // Trigger a single ranging measurement
        i2c_write_reg(VL53L0X_ADDR, SYSRANGE_START_REG, 0x01);
        vTaskDelay(50 / portTICK_PERIOD_MS); // Allow hardware ranging time

        // Fetch calculation register data
        if (i2c_read_reg(VL53L0X_ADDR, RESULT_RANGE_REG, dist_data, 2) == ESP_OK) {
            tof_distance_mm = ((dist_data[0] << 8) | dist_data[1]) / 2;
        } else {
            tof_distance_mm = 0; // Error or reading timeout
        }

        // --- 3. Output Unified Data Logs ---
        if (ir_raw_value > 3800) {
            ESP_LOGI(TAG, "IR Proximity: (%4d)", ir_raw_value);
        } else {
            ESP_LOGI(TAG, "IR Proximity: (%4d)", ir_raw_value);
        }

        if (tof_distance_mm > LOWER_THRESHOLD && tof_distance_mm < UPPER_THRESHOLD) {
            ESP_LOGI(TAG, "ToF Distance: %d mm", tof_distance_mm);
        } else {
            ESP_LOGW(TAG, "ToF Distance: Out of Range / Invalid Reading");
        }

        printf("-----------------------------------------------------------------\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
