#pragma once

#ifndef ULP_INA219_TYPES_H
#define ULP_INA219_TYPES_H

#define SAMPLING_DELAY_WAIT 75 * 1000
#define MAX_BUS 2

typedef enum { PRG_STATE_NONE = 0, PRG_STATE_RUNNING, PRG_STATE_SLEEPING } program_state_enum_t;

typedef struct {
  int8_t pin;
  uint8_t counter;
  uint8_t interval;
} activity_led_t;

typedef volatile struct {
  uint8_t address;
  uint8_t reset;
  float max_system_voltage;
  float max_system_current;
  float shunt_resistance;
  float current_accum_threshold;
  float power_accum_threshold;
  uint32_t calibration_register;
} bus_config_t;

typedef volatile struct {
  float voltage;
  float current;
  float power;
  float shunt_voltage;
  uint32_t calibration_register;
  uint32_t current_lsb;
  esp_err_t error_code;
  uint64_t last_sample_time;
  float charge_net;
  float energy_net;
  float voltage_min;
  float voltage_max;
  float current_min;
  float current_max;
  float power_min;
  float power_max;
} bus_values_t;

typedef volatile struct {
  bus_config_t config;
  bus_values_t values;
} bus_t;

typedef volatile struct {
  bus_t buses[MAX_BUS];
  activity_led_t led;
  uint64_t run_duration;
  uint32_t slow_clk_period;
  program_state_enum_t prg_state;
} ulp_ina219_context_t;

#endif  // ULP_INA219_TYPES_H