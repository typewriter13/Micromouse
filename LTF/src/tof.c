#include "../include/tof.h"

static esp_err_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_write_to_device(I2C_MASTER_NUM, dev_addr, write_buf, 2, 1000 / portTICK_PERIOD_MS);
}

static esp_err_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len) {
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
    i2c_param_config(I2C_MASTER_NUM, &i2c_config);
    i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0);

    i2c_write_reg(TOF_SENSOR_ADDR, TOF_SENSOR_REG_SYSRANGE_START, 0x02);
}

uint16_t tof_read_distance(void) {
    uint8_t dist_data[2];

    if (i2c_read_reg(TOF_SENSOR_ADDR, TOF_SENSOR_REG_RESULT_RANGE_VAL, dist_data, 2) == ESP_OK) {
        uint16_t dist = ((dist_data[0] << 8) | dist_data[1]) / 2;

        if (dist < 20 || dist > 1200) {
            return 9999;
        }
        return dist;
    }

    return 9999;
}
