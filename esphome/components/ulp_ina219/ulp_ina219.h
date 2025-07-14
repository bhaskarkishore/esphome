#pragma once

#ifndef ULP_INA219_H
#define ULP_INA219_H

#include "esphome/core/gpio.h"
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "ulp/common.h"

namespace esphome {
namespace ulp_ina219 {

enum UlpProgramLoadStatusEnum {
  ULP_PROGRAM_LOAD_NONE = 0,
  ULP_PROGRAM_LOAD_PREV,
  ULP_PROGRAM_LOAD_COMPLETE,
  ULP_PROGRAM_LOAD_ERROR
};

class UlpIna219 : public Component {
 public:
  UlpIna219() : Component() {
    // We initialize rtc memory here so that subsequent set_* fn
    // calls can write config directly to lp memory instead of
    // having to duplicate variables in hp memory and then
    // copy them during setup().
    ulp_program_load_status_ = lp_core_load_program_();
  }

  void setup() override;
  void dump_config() override;
  void on_powerdown() override;

  void set_address(uint8_t bus_idx, uint8_t address) {
    if (is_valid_bus(bus_idx)) {
      ulp_ina219_context_t *ctx = get_ulp_context();
      ctx->buses[bus_idx].config.address = address;
    }
  }

  void set_shunt_resistance(uint8_t bus_idx, float shunt_resistance) {
    if (is_valid_bus(bus_idx)) {
      ulp_ina219_context_t *ctx = get_ulp_context();
      ctx->buses[bus_idx].config.shunt_resistance = shunt_resistance;
    }
  }

  void set_max_system_voltage(uint8_t bus_idx, float max_system_voltage) {
    if (is_valid_bus(bus_idx)) {
      ulp_ina219_context_t *ctx = get_ulp_context();
      ctx->buses[bus_idx].config.max_system_voltage = max_system_voltage;
    }
  }

  void set_max_system_current(uint8_t bus_idx, float max_system_current) {
    if (is_valid_bus(bus_idx)) {
      ulp_ina219_context_t *ctx = get_ulp_context();
      ctx->buses[bus_idx].config.max_system_current = max_system_current;
    }
  }

  void set_current_accum_threshold(uint8_t bus_idx, float current_accum_threshold) {
    if (is_valid_bus(bus_idx)) {
      ulp_ina219_context_t *ctx = get_ulp_context();
      ctx->buses[bus_idx].config.current_accum_threshold = current_accum_threshold;
    }
  }

  void set_power_accum_threshold(uint8_t bus_idx, float power_accum_threshold) {
    if (is_valid_bus(bus_idx)) {
      ulp_ina219_context_t *ctx = get_ulp_context();
      ctx->buses[bus_idx].config.power_accum_threshold = power_accum_threshold;
    }
  }

  void set_calibration_register_override(uint8_t bus_idx, uint32_t calibration_register_override) {
    if (is_valid_bus(bus_idx)) {
      ulp_ina219_context_t *ctx = get_ulp_context();
      ctx->buses[bus_idx].config.calibration_register_override = calibration_register_override;
    }
  }

  void set_trigger(uint8_t bus_idx, uint8_t trg_idx, wake_trigger_mode_enum_t mode, uint32_t debounce_ms, float above,
                   float below, float threshold);

  void set_led_pin(InternalGPIOPin *led_pin) { led_pin_ = led_pin; }

  void set_led_interval(uint8_t interval) { led_interval_ = interval; }

  void set_sleep_duration(uint32_t sleep_duration) { sleep_duration_ = sleep_duration; }

  void set_net_charge(uint8_t bus_idx, float charge);

  void set_net_energy(uint8_t bus_idx, float energy);

  void reset_values(uint8_t bus_idx);

  void reset_triggers(uint8_t bus_idx);

  void enable_ulp_wake_src();

  void set_lp_sda_pin(InternalGPIOPin *lp_sda_pin) { lp_sda_pin_ = lp_sda_pin; }

  void set_lp_scl_pin(InternalGPIOPin *lp_scl_pin) { lp_scl_pin_ = lp_scl_pin; }

  static ulp_ina219_context_t *get_ulp_context();

 protected:
  InternalGPIOPin *lp_sda_pin_ = nullptr;
  InternalGPIOPin *lp_scl_pin_ = nullptr;
  UlpProgramLoadStatusEnum ulp_program_load_status_ = ULP_PROGRAM_LOAD_NONE;
  InternalGPIOPin *led_pin_ = nullptr;
  uint8_t led_interval_ = 0;
  uint32_t sleep_duration_ = 0;

  static bool wait_for_ulp_sleep();
  static bool is_valid_bus(uint8_t bus_idx);

  esp_err_t ulp_core_init_();
  esp_err_t ulp_rtc_io_init_();
  esp_err_t ulp_i2c_init_();
  UlpProgramLoadStatusEnum lp_core_load_program_();
};

}  // namespace ulp_ina219
}  // namespace esphome

#endif  // ULP_INA219_H
