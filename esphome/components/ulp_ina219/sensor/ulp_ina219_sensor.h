#pragma once

#ifndef ULP_INA219_SENSOR_H
#define ULP_INA219_SENSOR_H

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/ulp_ina219/ulp_ina219.h"

namespace esphome {
namespace ulp_ina219 {

class UlpIna219Sensor : public PollingComponent : public Parented<UlpIna219> {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage_sensor_ = voltage_sensor; }

  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }

  void set_shunt_voltage_sensor(sensor::Sensor *shunt_voltage_sensor) { shunt_voltage_sensor_ = shunt_voltage_sensor; }

  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }

  void set_charge_net_sensor(sensor::Sensor *charge_net_sensor) { charge_net_sensor_ = charge_net_sensor; }

  void set_energy_net_sensor(sensor::Sensor *energy_net_sensor) { energy_net_sensor_ = energy_net_sensor; }

  void set_charge_out_sensor(sensor::Sensor *charge_out_sensor) { charge_out_sensor_ = charge_out_sensor; }

  void set_energy_out_sensor(sensor::Sensor *energy_out_sensor) { energy_out_sensor_ = energy_out_sensor; }

  void set_charge_in_sensor(sensor::Sensor *charge_in_sensor) { charge_in_sensor_ = charge_in_sensor; }

  void set_energy_in_sensor(sensor::Sensor *energy_in_sensor) { energy_in_sensor_ = energy_in_sensor; }

  void set_voltage_min_sensor(sensor::Sensor *voltage_min_sensor) { voltage_min_sensor_ = voltage_min_sensor; }

  void set_voltage_max_sensor(sensor::Sensor *voltage_max_sensor) { voltage_max_sensor_ = voltage_max_sensor; }

  void set_current_min_sensor(sensor::Sensor *current_min_sensor) { current_min_sensor_ = current_min_sensor; }

  void set_current_max_sensor(sensor::Sensor *current_max_sensor) { current_max_sensor_ = current_max_sensor; }

  void set_power_min_sensor(sensor::Sensor *power_min_sensor) { power_min_sensor_ = power_min_sensor; }

  void set_power_max_sensor(sensor::Sensor *power_max_sensor) { power_max_sensor_ = power_max_sensor; }

  void set_bus_index(uint8_t bus_idx) { bus_index_ = is_valid_bus(bus_idx) ? bus_idx : -1; }

 protected:
  int8_t bus_index_ = -1;

  sensor::Sensor *voltage_sensor_ = nullptr;
  sensor::Sensor *current_sensor_ = nullptr;
  sensor::Sensor *shunt_voltage_sensor_ = nullptr;
  sensor::Sensor *power_sensor_ = nullptr;
  sensor::Sensor *charge_net_sensor_ = nullptr;
  sensor::Sensor *charge_in_sensor_ = nullptr;
  sensor::Sensor *charge_out_sensor_ = nullptr;
  sensor::Sensor *energy_net_sensor_ = nullptr;
  sensor::Sensor *energy_in_sensor_ = nullptr;
  sensor::Sensor *energy_out_sensor_ = nullptr;
  sensor::Sensor *voltage_min_sensor_ = nullptr;
  sensor::Sensor *voltage_max_sensor_ = nullptr;
  sensor::Sensor *current_min_sensor_ = nullptr;
  sensor::Sensor *current_max_sensor_ = nullptr;
  sensor::Sensor *power_min_sensor_ = nullptr;
  sensor::Sensor *power_max_sensor_ = nullptr;

  static bool is_valid_bus(uint8_t bus_idx);
};

}  // namespace ulp_ina219
}  // namespace esphome

#endif  // ULP_INA219_SENSOR_H
