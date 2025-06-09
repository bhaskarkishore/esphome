#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace ulp_ina219 {

class UlpIna219 : public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_voltage_sensor(uint8_t bus_idx, sensor::Sensor *voltage_sensor) { voltage_sensor_ = voltage_sensor; }
  void set_current_sensor(uint8_t bus_idx, sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
  void set_shunt_voltage_sensor(uint8_t bus_idx, sensor::Sensor *shunt_voltage_sensor) {
    shunt_voltage_sensor_ = shunt_voltage_sensor;
  }
  void set_power_sensor(uint8_t bus_idx, sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_charge_sensor(uint8_t bus_idx, sensor::Sensor *charge_sensor) { charge_sensor_ = charge_sensor; }
  void set_energy_sensor(uint8_t bus_idx, sensor::Sensor *energy_sensor) { energy_sensor_ = energy_sensor; }

  void set_duration_sensor(uint8_t bus_idx, sensor::Sensor *duration_sensor) { duration_sensor_ = duration_sensor; }

 protected:
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *shunt_voltage_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *charge_sensor_{nullptr};
  sensor::Sensor *energy_sensor_{nullptr};

  sensor::Sensor *duration_sensor_{nullptr};

  float read_voltage();
  float read_current();
};
}  // namespace ulp_ina219
}  // namespace esphome
