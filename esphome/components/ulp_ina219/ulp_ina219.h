#pragma once

#ifndef ULP_INA219_H
#define ULP_INA219_H

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "ulp/ulp_types.h"

namespace esphome {
namespace ulp_ina219 {

enum UlpProgramLoadStatusEnum {
  ULP_PROGRAM_LOAD_NONE = 0,
  ULP_PROGRAM_LOAD_PREV,
  ULP_PROGRAM_LOAD_COMPLETE,
  ULP_PROGRAM_LOAD_ERROR
};

class UlpIna219 : public PollingComponent {
 public:
  UlpIna219() : PollingComponent(0) {
    // We initialize rtc memory here so that subsequent set_* fn
    // calls can write config directly to lp memory instead of
    // having to duplicate variables in hp memory and then
    // copy them during setup().
    lp_program_load_status_ = lp_core_load_program_();
  }

  void setup() override;
  void update() override;
  void dump_config() override;
  void on_powerdown() override;

  void set_voltage_sensor(uint8_t bus_idx, sensor::Sensor *voltage_sensor) {
    if (is_valid_bus(bus_idx)) {
      voltage_sensor_[bus_idx] = voltage_sensor;
    }
  }

  void set_current_sensor(uint8_t bus_idx, sensor::Sensor *current_sensor) {
    if (is_valid_bus(bus_idx)) {
      current_sensor_[bus_idx] = current_sensor;
    }
  }

  void set_shunt_voltage_sensor(uint8_t bus_idx, sensor::Sensor *shunt_voltage_sensor) {
    if (is_valid_bus(bus_idx)) {
      shunt_voltage_sensor_[bus_idx] = shunt_voltage_sensor;
    }
  }

  void set_power_sensor(uint8_t bus_idx, sensor::Sensor *power_sensor) {
    if (is_valid_bus(bus_idx)) {
      power_sensor_[bus_idx] = power_sensor;
    }
  }

  void set_charge_net_sensor(uint8_t bus_idx, sensor::Sensor *charge_net_sensor) {
    if (is_valid_bus(bus_idx)) {
      charge_net_sensor_[bus_idx] = charge_net_sensor;
    }
  }

  void set_energy_net_sensor(uint8_t bus_idx, sensor::Sensor *energy_net_sensor) {
    if (is_valid_bus(bus_idx)) {
      energy_net_sensor_[bus_idx] = energy_net_sensor;
    }
  }

  void set_charge_out_sensor(uint8_t bus_idx, sensor::Sensor *charge_out_sensor) {
    if (is_valid_bus(bus_idx)) {
      charge_out_sensor_[bus_idx] = charge_out_sensor;
    }
  }

  void set_energy_out_sensor(uint8_t bus_idx, sensor::Sensor *energy_out_sensor) {
    if (is_valid_bus(bus_idx)) {
      energy_out_sensor_[bus_idx] = energy_out_sensor;
    }
  }

  void set_charge_in_sensor(uint8_t bus_idx, sensor::Sensor *charge_in_sensor) {
    if (is_valid_bus(bus_idx)) {
      charge_in_sensor_[bus_idx] = charge_in_sensor;
    }
  }

  void set_energy_in_sensor(uint8_t bus_idx, sensor::Sensor *energy_in_sensor) {
    if (is_valid_bus(bus_idx)) {
      energy_in_sensor_[bus_idx] = energy_in_sensor;
    }
  }

  void set_voltage_min_sensor(uint8_t bus_idx, sensor::Sensor *voltage_min_sensor) {
    if (is_valid_bus(bus_idx)) {
      voltage_min_sensor_[bus_idx] = voltage_min_sensor;
    }
  }

  void set_voltage_max_sensor(uint8_t bus_idx, sensor::Sensor *voltage_max_sensor) {
    if (is_valid_bus(bus_idx)) {
      voltage_max_sensor_[bus_idx] = voltage_max_sensor;
    }
  }

  void set_current_min_sensor(uint8_t bus_idx, sensor::Sensor *current_min_sensor) {
    if (is_valid_bus(bus_idx)) {
      current_min_sensor_[bus_idx] = current_min_sensor;
    }
  }
  void set_current_max_sensor(uint8_t bus_idx, sensor::Sensor *current_max_sensor) {
    if (is_valid_bus(bus_idx)) {
      current_max_sensor_[bus_idx] = current_max_sensor;
    }
  }

  void set_power_min_sensor(uint8_t bus_idx, sensor::Sensor *power_min_sensor) {
    if (is_valid_bus(bus_idx)) {
      power_min_sensor_[bus_idx] = power_min_sensor;
    }
  }

  void set_power_max_sensor(uint8_t bus_idx, sensor::Sensor *power_max_sensor) {
    if (is_valid_bus(bus_idx)) {
      power_max_sensor_[bus_idx] = power_max_sensor;
    }
  }

  void set_address(uint8_t bus_idx, uint8_t address) {
    if (is_valid_bus(bus_idx)) {
      bus_config_[bus_idx].address = address;
    }
  }

  void set_shunt_resistance(uint8_t bus_idx, float shunt_resistance) {
    if (is_valid_bus(bus_idx)) {
      bus_config_[bus_idx].shunt_resistance = shunt_resistance;
    }
  }

  void set_max_system_voltage(uint8_t bus_idx, float max_system_voltage) {
    if (is_valid_bus(bus_idx)) {
      bus_config_[bus_idx].max_system_voltage = max_system_voltage;
    }
  }

  void set_max_system_current(uint8_t bus_idx, float max_system_current) {
    if (is_valid_bus(bus_idx)) {
      bus_config_[bus_idx].max_system_current = max_system_current;
    }
  }

  void set_current_accum_threshold(uint8_t bus_idx, float current_accum_threshold) {
    if (is_valid_bus(bus_idx)) {
      bus_config_[bus_idx].current_accum_threshold = current_accum_threshold;
    }
  }

  void set_power_accum_threshold(uint8_t bus_idx, float power_accum_threshold) {
    if (is_valid_bus(bus_idx)) {
      bus_config_[bus_idx].power_accum_threshold = power_accum_threshold;
    }
  }

  void set_calibration_register_override(uint8_t bus_idx, uint32_t calibration_register_override) {
    if (is_valid_bus(bus_idx)) {
      bus_config_[bus_idx].calibration_register_override = calibration_register_override;
    }
  }

  void set_trigger(uint8_t bus_idx, uint8_t trg_idx, wake_trigger_mode_enum_t mode, uint32_t debounce_ms, float above,
                   float below, float threshold);

  void set_led_pin(InternalGPIOPin *led_pin) { led_pin_ = gpio_num_t(led_pin->get_pin()); }

  void set_led_interval(uint8_t interval) { led_interval_ = interval; }

  void set_sleep_duration(uint32_t sleep_duration) { sleep_duration_ = sleep_duration; }

  void set_net_charge(uint8_t bus_idx, float charge);

  void set_net_energy(uint8_t bus_idx, float energy);

  void reset_values(uint8_t bus_idx);

  void reset_triggers(uint8_t bus_idx);

  void enable_ulp_wake_src();

  void set_lp_sda_pin(InternalGPIOPin *lp_sda_pin) { lp_sda_pin_ = gpio_num_t(lp_sda_pin->get_pin()); }

  void set_lp_scl_pin(InternalGPIOPin *lp_scl_pin) { lp_scl_pin_ = gpio_num_t(lp_scl_pin->get_pin()); }

  void set_lp_sda_pullup_en(bool lp_sda_pullup_en) { lp_sda_pullup_en_ = lp_sda_pullup_en; }

  void set_lp_scl_pullup_en(bool lp_scl_pullup_en) { lp_scl_pullup_en_ = lp_scl_pullup_en; }

 protected:
  gpio_num_t lp_sda_pin_ = GPIO_NUM_NC;
  gpio_num_t lp_scl_pin_ = GPIO_NUM_NC;
  bool lp_sda_pullup_en_ = true;
  bool lp_scl_pullup_en_ = true;
  UlpProgramLoadStatusEnum lp_program_load_status_ = ULP_PROGRAM_LOAD_NONE;

  gpio_num_t led_pin_ = GPIO_NUM_NC;
  uint8_t led_interval_ = 0;
  uint32_t sleep_duration_ = 0;
  BusConfigStruct bus_config_[MAX_BUS] = {};

  static ulp_ina219_context_t *get_ulp_context();
  static bool wait_for_ulp_sleep();
  static bool is_valid_bus(uint8_t bus_idx);

  sensor::Sensor *voltage_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *current_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *shunt_voltage_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *power_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *charge_net_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *charge_in_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *charge_out_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *energy_net_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *energy_in_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *energy_out_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *voltage_min_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *voltage_max_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *current_min_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *current_max_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *power_min_sensor_[MAX_BUS] = {nullptr, nullptr};
  sensor::Sensor *power_max_sensor_[MAX_BUS] = {nullptr, nullptr};

  esp_err_t ulp_core_init_();
  esp_err_t ulp_rtc_io_init_();
  esp_err_t ulp_i2c_init_();
  UlpProgramLoadStatusEnum lp_core_load_program_();
};

template<typename... Ts> class SetNetChargeAction : public Action<Ts...>, public Parented<UlpIna219> {
 public:
  TEMPLATABLE_VALUE(uint8_t, bus_idx)
  TEMPLATABLE_VALUE(float, value)

  void play(Ts... x) override { this->parent_->set_net_charge(this->bus_idx_.value(x...), this->value_.value(x...)); }
};

template<typename... Ts> class SetNetEnergyAction : public Action<Ts...>, public Parented<UlpIna219> {
 public:
  TEMPLATABLE_VALUE(uint8_t, bus_idx)
  TEMPLATABLE_VALUE(float, value)

  void play(Ts... x) override { this->parent_->set_net_energy(this->bus_idx_.value(x...), this->value_.value(x...)); }
};

template<typename... Ts> class ResetValuesAction : public Action<Ts...>, public Parented<UlpIna219> {
 public:
  TEMPLATABLE_VALUE(uint8_t, bus_idx)

  void play(Ts... x) override { this->parent_->reset_values(this->bus_idx_.value(x...)); }
};

template<typename... Ts> class ResetTriggersAction : public Action<Ts...>, public Parented<UlpIna219> {
 public:
  TEMPLATABLE_VALUE(uint8_t, bus_idx)

  void play(Ts... x) override { this->parent_->reset_triggers(this->bus_idx_.value(x...)); }
};

}  // namespace ulp_ina219
}  // namespace esphome

#endif
