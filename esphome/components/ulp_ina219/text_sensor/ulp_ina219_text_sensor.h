#pragma once

#ifndef ULP_INA219_SENSOR_H
#define ULP_INA219_SENSOR_H

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/ulp_ina219/ulp_ina219.h"

namespace esphome {
namespace ulp_ina219 {

class UlpIna219TextSensor : public text_sensor::TextSensor, public Component, public Parented<UlpIna219> {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_trigger(uint8_t bus_idx, uint8_t trg_idx, wake_trigger_mode_enum_t mode, uint32_t debounce_ms, float above,
                   float below, float threshold) {
    this->parent_->set_trigger(bus_idx, trg_idx, mode, debounce_ms, above, below, threshold);
  }

 protected:
  static bool is_valid_bus(uint8_t bus_idx);
};

}  // namespace ulp_ina219
}  // namespace esphome

#endif  // ULP_INA219_SENSOR_H
