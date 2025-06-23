#pragma once

#ifndef ULP_INA219_TYPES_H
#define ULP_INA219_TYPES_H

typedef struct {
  uint32_t slow_clk_period;
  uint64_t run_duration;
  uint8_t prg_state;
  uint8_t led_interval_counter;
} state_t;

typedef struct {
  uint8_t led_interval;
  int8_t led_gpio;
} general_cfg_t;

typedef struct {
  uint8_t address;
  uint8_t reset;
  float max_system_voltage;
  float max_system_current;
  float shunt_resistance;
  float current_accum_threshold;
  float power_accum_threshold;
  uint32_t calibration_register;
} bus_cfg_t;

typedef struct {
  float voltage;
  float current;
  float power;
  float shunt_voltage;
  uint32_t calibration_register;
  uint32_t current_lsb;
  esp_err_t error_code;
  uint64_t last_sample_time;
  float charge;
  float energy;
  float voltage_min;
  float voltage_max;
  float current_min;
  float current_max;
  float power_min;
  float power_max;
} bus_values_t;

#endif  // ULP_INA219_TYPES_H