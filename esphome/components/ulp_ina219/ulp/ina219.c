#include "ina219.h"
#include "i2c.h"

// Some insipration and reference taken from https://github.com/esphome/esphome/tree/dev/esphome/components/ina219

static inline int32_t _ceilf(float x) { return (int32_t) (x + 0.999999f); }

esp_err_t ina219_power_down(uint8_t address) { return i2c_write16(address, INA219_REGISTER_CONFIG, 0x0); }

esp_err_t ina219_init(uint8_t address, float max_voltage, float r_shunt, float max_current,
                      uint32_t cfg_calibration_register, uint32_t *calibration_register, uint32_t *current_lsb) {
  esp_err_t ret = ESP_OK;
  ret = i2c_write16(address, INA219_REGISTER_CONFIG, 0x8000);

  if (ret != ESP_OK) {
    return ret;
  }

  ulp_lp_core_delay_us(1 * 1000);

  // Continuous operation of Bus and Shunt ADCs
  uint16_t config = 0b0000000000000111;
  // Bus ADC 12 bit+128 samples
  config |= 0b0000011110000000;
  // Shunt ADC 12 bit+128 samples
  config |= 0b0000000001111000;

  // 16v range
  // For higher voltages, adc precision will halve
  if (max_voltage > 16.f) {
    config |= 0b0010000000000000;
  } else {
    config |= 0b0000000000000000;
  }

  // 40mv gain
  float max_shunt_voltage = max_current * 1000.f * r_shunt;
  uint16_t pga_gain = 0b00;

  if (max_shunt_voltage < 40.f) {
    pga_gain = 0b00;  // 40mV
  } else if (max_shunt_voltage <= 0.04f) {
    pga_gain = 0b01;  // 80mV
  } else if (max_shunt_voltage <= 0.08f) {
    pga_gain = 0b10;  // 160mV
  } else {
    pga_gain = 0b11;  // 320mV
  }
  config |= pga_gain << 11;

  ret = i2c_write16(address, INA219_REGISTER_CONFIG, config);

  if (ret != ESP_OK) {
    return ret;
  }

  *current_lsb = (uint32_t) _ceilf(max_current * 1000000.0f / 0x8000);
  if (cfg_calibration_register != 0) {
    *calibration_register = cfg_calibration_register;
  } else {
    *calibration_register = (uint16_t) (0.04096f / (0.000001 * (*current_lsb) * r_shunt));
  }

  ret = i2c_write16(address, INA219_REGISTER_CALIBRATION, *calibration_register);

  return ret;
}

esp_err_t ina219_bus_voltage(uint8_t address, float *bus_voltage) {
  uint16_t raw_bus_voltage = 0;
  esp_err_t ret = i2c_read16(address, INA219_REGISTER_BUS_VOLTAGE, &raw_bus_voltage);
  if (ret == ESP_OK) {
    raw_bus_voltage >>= 3;
    *bus_voltage = raw_bus_voltage * 0.004f;
  }
  return ret;
}

esp_err_t ina219_current(uint8_t address, float *current, uint32_t current_lsb) {
  uint16_t raw_current = 0;
  esp_err_t ret = i2c_read16(address, INA219_REGISTER_CURRENT, &raw_current);
  if (ret == ESP_OK) {
    // raw_current * current_lsb is in microamps
    *current = (((int16_t) raw_current) * (current_lsb / 1000.f)) / 1000.f;
  }
  return ret;
}

esp_err_t ina219_shunt_voltage(uint8_t address, float *shunt_voltage) {
  uint16_t raw_shunt_voltage = 0;
  esp_err_t ret = i2c_read16(address, INA219_REGISTER_SHUNT_VOLTAGE, &raw_shunt_voltage);
  if (ret == ESP_OK) {
    *shunt_voltage = ((int16_t) raw_shunt_voltage) * 10.f / 1000000.f;
  }
  return ret;
}

esp_err_t ina219_power(uint8_t address, float *power, uint32_t current_lsb) {
  uint16_t raw_power = 0;
  esp_err_t ret = i2c_read16(address, INA219_REGISTER_POWER, &raw_power);
  if (ret == ESP_OK) {
    *power = ((int16_t) raw_power) * current_lsb * 20.f / 1000000.f;
  }
  return ret;
}