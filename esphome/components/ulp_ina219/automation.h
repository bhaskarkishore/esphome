#pragma once

#ifndef ULP_INA219_AUTOMATION_H
#define ULP_INA219_AUTOMATION_H

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "ulp_ina219.h"

namespace esphome {
namespace ulp_ina219 {

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

#endif  // ULP_INA219_AUTOMATION_H
