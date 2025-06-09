#include "i2c.h"

esp_err_t i2c_write(uint16_t address, uint8_t *data, size_t len) {
  return lp_core_i2c_master_write_to_device(LP_I2C_NUM_0, address, data, len, LP_I2C_TRANS_TIMEOUT_LP_CYCLES);
}

esp_err_t i2c_read(uint16_t address, uint8_t *data, size_t len) {
  return lp_core_i2c_master_read_from_device(LP_I2C_NUM_0, address, data, len, LP_I2C_TRANS_TIMEOUT_LP_CYCLES);
}

esp_err_t i2c_write16(uint16_t address, uint8_t reg, uint16_t data) {
  uint8_t data_wr[3];
  data_wr[0] = reg;
  data_wr[1] = (data >> 8) & 0xFF;
  data_wr[2] = data & 0xFF;
  return i2c_write(address, data_wr, 3);
}

esp_err_t i2c_read16(uint16_t address, uint8_t reg, uint16_t *data) {
  esp_err_t ret = i2c_write(address, &reg, 1);
  if (ret != ESP_OK) {
    return ret;
  }
  uint8_t data_rd[2];
  ret = i2c_read(address, data_rd, 2);
  if (ret == ESP_OK) {
    *data = data_rd[0] << 8 | data_rd[1];
  }
  return ret;
}
