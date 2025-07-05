#pragma once

#ifndef ULPINA219_ULPTYPES_H
#define ULPINA219_ULPTYPES_H

#include "esp_err.h"

#ifdef __cplusplus
namespace esphome {
namespace ulp_ina219 {
#endif

#define MAX_BUS (2)
#define MAX_TRIGGERS (3)
static const uint32_t SAMPLING_DELAY_WAIT = 75 * 1000;

// The following code is shared between the ulp and hp cpus.
// The ulp code is compiled using a C compiler where modern
// language features may not be supported.
// NOLINTBEGIN(modernize-use-using)
typedef enum { PRG_STATE_NONE = 0, PRG_STATE_RUN, PRG_STATE_WAIT_SLEEP } program_state_enum_t;

typedef enum {
  TRIG_MODE_NONE = 0,
  TRIG_MODE_VOLTAGE,
  TRIG_MODE_CURRENT,
  TRIG_MODE_CHARGE,
  TRIG_MODE_ENERGY,
  TRIG_MODE_CHARGE_DELTA,
  TRIG_MODE_ENERGY_DELTA
} wake_trigger_mode_enum_t;

typedef enum { TRIG_NOT_SET = 0, TRIG_SET, TRIG_TRIGGERED } wake_trigger_state_enum_t;

typedef struct {
  wake_trigger_mode_enum_t mode;
  wake_trigger_state_enum_t status;
  union {
    struct {
      float above;
      float below;
    } range;
    struct {
      double current;
      double threshold;
    } accum;
  } conditions;
} wake_trigger_t;

typedef struct {
  int8_t pin;
  uint8_t counter;
  uint8_t interval;
} activity_led_t;

typedef volatile struct {
  uint32_t calibration_register;
  float max_system_voltage;
  float max_system_current;
  float shunt_resistance;
  float current_accum_threshold;
  float power_accum_threshold;
  uint8_t address;
  uint8_t reset;
} bus_config_t;

typedef volatile struct {
  uint64_t last_sample_time;
  double charge_net;
  double charge_in;
  double charge_out;
  double energy_net;
  double energy_in;
  double energy_out;
  uint32_t calibration_register;
  uint32_t current_lsb;
  esp_err_t error_code;
  float voltage;
  float current;
  float power;
  float shunt_voltage;
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
  wake_trigger_t triggers[MAX_TRIGGERS];
} bus_t;

typedef volatile struct {
  bus_t buses[MAX_BUS];
  activity_led_t led;
  uint64_t run_duration;
  uint32_t slow_clk_period;
  program_state_enum_t prg_state;
  bool main_cpu_awake;
} ulp_ina219_context_t;

// NOLINTEND(modernize-use-using)

#ifdef __cplusplus
}  // namespace ulp_ina219
}  // namespace esphome
#endif

#endif  // ULPINA219_ULPTYPES_H
