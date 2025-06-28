#pragma once

#ifndef ULPINA219_ULP_I2C_H
#define ULPINA219_ULP_I2C_H

#include "esp_err.h"

static const int32_t LP_I2C_TRANS_TIMEOUT_LP_CYCLES = 5000;

esp_err_t i2c_write(uint16_t address, uint8_t *data, size_t len);

esp_err_t i2c_read(uint16_t address, uint8_t *data, size_t len);

esp_err_t i2c_write16(uint16_t address, uint8_t reg, uint16_t data);

esp_err_t i2c_read16(uint16_t address, uint8_t reg, uint16_t *data);

#endif  // ULPINA219_ULP_I2C_H
