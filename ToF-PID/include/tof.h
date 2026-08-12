#ifndef TOF_H
#define TOF_H
#include "driver/i2c.h"

#define I2C_MASTER_NUM         I2C_NUM_0
#define I2C_MASTER_SDA_IO      41
#define I2C_MASTER_SCL_IO      42
#define I2C_MASTER_FREQ_HZ     100000

#define TOF_SENSOR_ADDR        0x29
#define TOF_SENSOR_REG_SYSRANGE_START 0x00
#define TOF_SENSOR_REG_RESULT_RANGE_VAL 0x1E

void tof_i2c_init(void);
esp_err_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data);
esp_err_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len);
uint16_t tof_read_distance(void);

#endif
