#pragma once

#ifndef ULP_INA219_TEXT_SENSOR_H
#define ULP_INA219_TEXT_SENSOR_H

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/ulp_ina219/ulp_ina219.h"

namespace esphome {
namespace ulp_ina219 {

class UlpIna219TextSensor : public text_sensor::TextSensor, public PollingComponent, public Parented<UlpIna219> {
 public:
  void update() override;
  void setup() override;
  void dump_config() override;

  void set_alarm(uint8_t bus_idx, uint8_t alarm_idx, alarm_mode_enum_t mode, uint32_t debounce_ms, float above,
                 float below, float threshold) {
    bus_idx_ = bus_idx;
    alarm_idx_ = alarm_idx;
    this->parent_->set_alarm(bus_idx, alarm_idx, mode, debounce_ms, above, below, threshold);
  }

 protected:
  uint8_t alarm_idx_;
  uint8_t bus_idx_;
  alarm_status_enum_t last_status_{ALARM_NOT_SET};
};

}  // namespace ulp_ina219
}  // namespace esphome

#endif  // ULP_INA219_TEXT_SENSOR_H
