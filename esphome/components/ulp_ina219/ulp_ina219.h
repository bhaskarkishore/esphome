#pragma once

#ifndef ULP_INA219_H
#define ULP_INA219_H

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "ulp/ulp_types.h"

namespace esphome {
namespace ulp_ina219 {

static const char *const TAG = "ulp_219";

class UlpIna219 : public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  void on_powerdown() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_voltage_sensor(uint8_t bus_idx, sensor::Sensor *voltage_sensor) {
    if (bus_idx < MAX_BUS) {
      voltage_sensor_[bus_idx] = voltage_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_current_sensor(uint8_t bus_idx, sensor::Sensor *current_sensor) {
    if (bus_idx < MAX_BUS) {
      current_sensor_[bus_idx] = current_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_shunt_voltage_sensor(uint8_t bus_idx, sensor::Sensor *shunt_voltage_sensor) {
    if (bus_idx < MAX_BUS) {
      shunt_voltage_sensor_[bus_idx] = shunt_voltage_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_power_sensor(uint8_t bus_idx, sensor::Sensor *power_sensor) {
    if (bus_idx < MAX_BUS) {
      power_sensor_[bus_idx] = power_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_charge_net_sensor(uint8_t bus_idx, sensor::Sensor *charge_net_sensor) {
    if (bus_idx < MAX_BUS) {
      charge_net_sensor_[bus_idx] = charge_net_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_energy_net_sensor(uint8_t bus_idx, sensor::Sensor *energy_net_sensor) {
    if (bus_idx < MAX_BUS) {
      energy_net_sensor_[bus_idx] = energy_net_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_charge_out_sensor(uint8_t bus_idx, sensor::Sensor *charge_out_sensor) {
    if (bus_idx < MAX_BUS) {
      charge_out_sensor_[bus_idx] = charge_out_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_energy_out_sensor(uint8_t bus_idx, sensor::Sensor *energy_out_sensor) {
    if (bus_idx < MAX_BUS) {
      energy_out_sensor_[bus_idx] = energy_out_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_charge_in_sensor(uint8_t bus_idx, sensor::Sensor *charge_in_sensor) {
    if (bus_idx < MAX_BUS) {
      charge_in_sensor_[bus_idx] = charge_in_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_energy_in_sensor(uint8_t bus_idx, sensor::Sensor *energy_in_sensor) {
    if (bus_idx < MAX_BUS) {
      energy_in_sensor_[bus_idx] = energy_in_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_voltage_min_sensor(uint8_t bus_idx, sensor::Sensor *voltage_min_sensor) {
    if (bus_idx < MAX_BUS) {
      voltage_min_sensor_[bus_idx] = voltage_min_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_voltage_max_sensor(uint8_t bus_idx, sensor::Sensor *voltage_max_sensor) {
    if (bus_idx < MAX_BUS) {
      voltage_max_sensor_[bus_idx] = voltage_max_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_current_min_sensor(uint8_t bus_idx, sensor::Sensor *current_min_sensor) {
    if (bus_idx < MAX_BUS) {
      current_min_sensor_[bus_idx] = current_min_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }
  void set_current_max_sensor(uint8_t bus_idx, sensor::Sensor *current_max_sensor) {
    if (bus_idx < MAX_BUS) {
      current_max_sensor_[bus_idx] = current_max_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_power_min_sensor(uint8_t bus_idx, sensor::Sensor *power_min_sensor) {
    if (bus_idx < MAX_BUS) {
      power_min_sensor_[bus_idx] = power_min_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_power_max_sensor(uint8_t bus_idx, sensor::Sensor *power_max_sensor) {
    if (bus_idx < MAX_BUS) {
      power_max_sensor_[bus_idx] = power_max_sensor;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_address(uint8_t bus_idx, uint8_t address) {
    if (bus_idx < MAX_BUS) {
      bus_config_[bus_idx].address = address;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_shunt_resistance(uint8_t bus_idx, float shunt_resistance) {
    if (bus_idx < MAX_BUS) {
      bus_config_[bus_idx].shunt_resistance = shunt_resistance;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_max_system_voltage(uint8_t bus_idx, float max_system_voltage) {
    if (bus_idx < MAX_BUS) {
      bus_config_[bus_idx].max_system_voltage = max_system_voltage;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_max_system_current(uint8_t bus_idx, float max_system_current) {
    if (bus_idx < MAX_BUS) {
      bus_config_[bus_idx].max_system_current = max_system_current;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_current_accum_threshold(uint8_t bus_idx, float current_accum_threshold) {
    if (bus_idx < MAX_BUS) {
      bus_config_[bus_idx].current_accum_threshold = current_accum_threshold;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_power_accum_threshold(uint8_t bus_idx, float power_accum_threshold) {
    if (bus_idx < MAX_BUS) {
      bus_config_[bus_idx].power_accum_threshold = power_accum_threshold;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_calibration_register(uint8_t bus_idx, uint32_t calibration_register) {
    if (bus_idx < MAX_BUS) {
      bus_config_[bus_idx].calibration_register = calibration_register;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_bus_enabled(uint8_t bus_idx) {
    if (bus_idx < MAX_BUS) {
      bus_enabled_[bus_idx] = true;
    } else {
      ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    }
  }

  void set_led_pin(InternalGPIOPin *led_pin) { led_pin_ = gpio_num_t(led_pin->get_pin()); }

  void set_led_interval(uint8_t interval) { led_interval_ = interval; }

  void set_sleep_duration(uint32_t sleep_duration) { sleep_duration_ = sleep_duration; }

  void set_net_charge(uint8_t bus_idx, double charge);

  void set_net_energy(uint8_t bus_idx, double energy);

  void reset_values(uint8_t bus_idx);

  void set_lp_sda_pin(InternalGPIOPin *lp_sda_pin) { lp_sda_pin_ = gpio_num_t(lp_sda_pin->get_pin()); }

  void set_lp_scl_pin(InternalGPIOPin *lp_scl_pin) { lp_scl_pin_ = gpio_num_t(lp_scl_pin->get_pin()); }

  void set_lp_sda_pullup_en(bool lp_sda_pullup_en) { lp_sda_pullup_en_ = lp_sda_pullup_en; }

  void set_lp_scl_pullup_en(bool lp_scl_pullup_en) { lp_scl_pullup_en_ = lp_scl_pullup_en; }

 protected:
  gpio_num_t lp_sda_pin_ = GPIO_NUM_NC;
  gpio_num_t lp_scl_pin_ = GPIO_NUM_NC;
  bool lp_sda_pullup_en_ = true;
  bool lp_scl_pullup_en_ = true;

  gpio_num_t led_pin_ = GPIO_NUM_NC;
  uint8_t led_interval_ = 0;
  uint32_t sleep_duration_ = 0;
  bool bus_enabled_[MAX_BUS] = {false, false};
  bus_config_struct bus_config_[MAX_BUS] = {0};

  static ulp_ina219_context_t *get_ulp_context();

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

  esp_err_t lp_core_init_();
  esp_err_t lp_rtc_io_init_();
  esp_err_t lp_i2c_init_();
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

}  // namespace ulp_ina219
}  // namespace esphome

#endif
