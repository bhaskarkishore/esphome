#pragma once

#include "ulp_lp_core_utils.h"
#include "ulp_lp_core_i2c.h"

#define INA219_REGISTER_CONFIG 0x0
#define INA219_REGISTER_SHUNT_VOLTAGE 0x01
#define INA219_REGISTER_BUS_VOLTAGE 0x02
#define INA219_REGISTER_POWER 0x03
#define INA219_REGISTER_CURRENT 0x04
#define INA219_REGISTER_CALIBRATION 0x05

esp_err_t ina219_init(uint8_t address, float max_voltage, float r_shunt, float max_current,
                      uint32_t cfg_calibration_register, uint32_t *calibration_register, uint32_t *current_lsb);

esp_err_t ina219_power_down(uint8_t address);

esp_err_t ina219_bus_voltage(uint8_t address, volatile float *bus_voltage);

esp_err_t ina219_current(uint8_t address, volatile float *current, uint32_t current_lsb);

esp_err_t ina219_shunt_voltage(uint8_t address, volatile float *shunt_voltage);

esp_err_t ina219_power(uint8_t address, volatile float *power, uint32_t current_lsb);