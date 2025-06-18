#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace ulp_ina219 {

#define MAX_BUSES 2

class UlpIna219 : public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_voltage_sensor(uint8_t bus_idx, sensor::Sensor *voltage_sensor) {
    if (bus_idx < MAX_BUSES) {
      voltage_sensor_[bus_idx] = voltage_sensor;
    }
  }

  void set_current_sensor(uint8_t bus_idx, sensor::Sensor *current_sensor) {
    if (bus_idx < MAX_BUSES) {
      current_sensor_[bus_idx] = current_sensor;
    }
  }

  void set_shunt_voltage_sensor(uint8_t bus_idx, sensor::Sensor *shunt_voltage_sensor) {
    if (bus_idx < MAX_BUSES) {
      shunt_voltage_sensor_[bus_idx] = shunt_voltage_sensor;
    }
  }

  void set_power_sensor(uint8_t bus_idx, sensor::Sensor *power_sensor) {
    if (bus_idx < MAX_BUSES) {
      power_sensor_[bus_idx] = power_sensor;
    }
  }

  void set_charge_sensor(uint8_t bus_idx, sensor::Sensor *charge_sensor) {
    if (bus_idx < MAX_BUSES) {
      charge_sensor_[bus_idx] = charge_sensor;
    }
  }

  void set_energy_sensor(uint8_t bus_idx, sensor::Sensor *energy_sensor) {
    if (bus_idx < MAX_BUSES) {
      energy_sensor_[bus_idx] = energy_sensor;
    }
  }

  void set_voltage_min_sensor(uint8_t bus_idx, sensor::Sensor *voltage_min_sensor) {
    if (bus_idx < MAX_BUSES) {
      voltage_min_sensor_[bus_idx] = voltage_min_sensor;
    }
  }

  void set_voltage_max_sensor(uint8_t bus_idx, sensor::Sensor *voltage_max_sensor) {
    if (bus_idx < MAX_BUSES) {
      voltage_max_sensor_[bus_idx] = voltage_max_sensor;
    }
  }

  void set_current_min_sensor(uint8_t bus_idx, sensor::Sensor *current_min_sensor) {
    if (bus_idx < MAX_BUSES) {
      current_min_sensor_[bus_idx] = current_min_sensor;
    }
  }
  void set_current_max_sensor(uint8_t bus_idx, sensor::Sensor *current_max_sensor) {
    if (bus_idx < MAX_BUSES) {
      current_max_sensor_[bus_idx] = current_max_sensor;
    }
  }

  void set_power_min_sensor(uint8_t bus_idx, sensor::Sensor *power_min_sensor) {
    if (bus_idx < MAX_BUSES) {
      power_min_sensor_[bus_idx] = power_min_sensor;
    }
  }

  void set_power_max_sensor(uint8_t bus_idx, sensor::Sensor *power_max_sensor) {
    if (bus_idx < MAX_BUSES) {
      power_max_sensor_[bus_idx] = power_max_sensor;
    }
  }

  void set_address(uint8_t bus_idx, uint8_t address) {
    if (bus_idx < MAX_BUSES) {
      address_[bus_idx] = address;
    }
  }

  void set_shunt_resistance(uint8_t bus_idx, float shunt_resistance) {
    if (bus_idx < MAX_BUSES) {
      shunt_resistance_[bus_idx] = shunt_resistance;
    }
  }

  void set_max_system_voltage(uint8_t bus_idx, float max_system_voltage) {
    if (bus_idx < MAX_BUSES) {
      max_system_voltage_[bus_idx] = max_system_voltage;
    }
  }

  void set_max_system_current(uint8_t bus_idx, float max_system_current) {
    if (bus_idx < MAX_BUSES) {
      max_system_current_[bus_idx] = max_system_current;
    }
  }

  void set_current_accum_threshold(uint8_t bus_idx, float current_clamp_threshold) {
    if (bus_idx < MAX_BUSES) {
      current_accum_threshold_[bus_idx] = current_clamp_threshold;
    }
  }

  void set_power_accum_threshold(uint8_t bus_idx, float power_clamp_threshold) {
    if (bus_idx < MAX_BUSES) {
      power_accum_threshold_[bus_idx] = power_clamp_threshold;
    }
  }

  void set_bus_enabled(uint8_t bus_idx) {
    if (bus_idx < MAX_BUSES) {
      bus_enabled_[bus_idx] = true;
    }
  }

  void set_led_pin(InternalGPIOPin *led_pin) { led_pin_ = gpio_num_t(led_pin->get_pin()); }

  void set_led_interval(uint8_t interval) { led_interval_ = interval; }

  void set_sleep_duration(uint32_t sleep_duration) { sleep_duration_ = sleep_duration; }

  void set_calibration_register(uint8_t bus_idx, uint32_t calibration_register) {
    if (bus_idx < MAX_BUSES) {
      calibration_register_[bus_idx] = calibration_register;
    }
  }

  void set_lp_sda_pin(InternalGPIOPin *lp_sda_pin) { lp_sda_pin_ = gpio_num_t(lp_sda_pin->get_pin()); }

  void set_lp_scl_pin(InternalGPIOPin *lp_scl_pin) { lp_scl_pin_ = gpio_num_t(lp_scl_pin->get_pin()); }

  void set_lp_sda_pullup_en(bool lp_sda_pullup_en) { lp_sda_pullup_en_ = lp_sda_pullup_en; }

  void set_lp_scl_pullup_en(bool lp_scl_pullup_en) { lp_scl_pullup_en_ = lp_scl_pullup_en; }

 protected:
  gpio_num_t lp_sda_pin_ = GPIO_NUM_6;  // Fixed pins for c6
  gpio_num_t lp_scl_pin_ = GPIO_NUM_7;  // Fixed pins for c6
  bool lp_sda_pullup_en_ = true;
  bool lp_scl_pullup_en_ = true;

  gpio_num_t led_pin_ = GPIO_NUM_NC;
  uint8_t led_interval_ = 0;
  uint32_t sleep_duration_ = 0;
  bool bus_enabled_[MAX_BUSES] = {false, false};
  uint8_t address_[MAX_BUSES] = {0x0, 0x0};
  float shunt_resistance_[MAX_BUSES] = {0.f, 0.f};
  float max_system_voltage_[MAX_BUSES] = {0.f, 0.f};
  float max_system_current_[MAX_BUSES] = {0.f, 0.f};
  float current_accum_threshold_[MAX_BUSES] = {0.f, 0.f};
  float power_accum_threshold_[MAX_BUSES] = {0.f, 0.f};
  float calibration_register_[MAX_BUSES] = {0, 0};

  sensor::Sensor *voltage_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *current_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *shunt_voltage_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *power_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *charge_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *energy_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *voltage_min_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *voltage_max_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *current_min_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *current_max_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *power_min_sensor_[MAX_BUSES] = {nullptr, nullptr};
  sensor::Sensor *power_max_sensor_[MAX_BUSES] = {nullptr, nullptr};

  esp_err_t lp_core_init_();
  esp_err_t lp_i2c_init_();
};
}  // namespace ulp_ina219
}  // namespace esphome
