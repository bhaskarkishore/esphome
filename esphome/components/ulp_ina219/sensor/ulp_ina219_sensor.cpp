#include "esphome/components/ulp_ina219/sensor/ulp_ina219_sensor.h"
#include "esphome/components/ulp_ina219/ulp_ina219.h"

namespace esphome {
namespace ulp_ina219 {

static const char *const TAG = "ulp_ina219.sensor";

bool UlpIna219Sensor::is_valid_bus(uint8_t bus_idx) {
  if (bus_idx < MAX_BUS) {
    return true;
  } else {
    ESP_LOGE(TAG, "Invalid bus idx %u", bus_idx);
    return false;
  }
}

void UlpIna219Sensor::setup() {
  if (!is_valid_bus(this->bus_index_)) {
    this->mark_failed("invalid bus idx");
  }

  volatile ulp_ina219_context_t *ctx = UlpIna219::get_ulp_context();
  volatile bus_config_t *c = &ctx->buses[this->bus_index_].config;

  if (!(c->address > 0)) {
    this->mark_failed("bus disabled");
  }
}

#define ULP_INA219_PUBLISH(name) \
  if ((this->name##_sensor_) != nullptr) { \
    (this->name##_sensor_)->publish_state(v->name); \
  }

void UlpIna219Sensor::update() {
  volatile ulp_ina219_context_t *ctx = UlpIna219::get_ulp_context();

  volatile bus_values_t *v = &ctx->buses[this->bus_index_].values;

  ESP_LOGD(TAG,
           "Bus index: %u\n"
           "Calibration register: %u\n"
           "Current lsb: %u"
           "Error code: %d\n",
           this->bus_index_, v->calibration_register, v->current_lsb, v->error_code);

  if (v->error_code != ESP_OK || this->parent_->is_failed()) {
    this->mark_failed("error reading device");
    return;
  }

  ULP_INA219_PUBLISH(charge_in);
  ULP_INA219_PUBLISH(charge_net);
  ULP_INA219_PUBLISH(charge_out);
  ULP_INA219_PUBLISH(current);
  ULP_INA219_PUBLISH(current_max);
  ULP_INA219_PUBLISH(current_min);
  ULP_INA219_PUBLISH(energy_in);
  ULP_INA219_PUBLISH(energy_net);
  ULP_INA219_PUBLISH(energy_out);
  ULP_INA219_PUBLISH(power);
  ULP_INA219_PUBLISH(power_max);
  ULP_INA219_PUBLISH(power_min);
  ULP_INA219_PUBLISH(shunt_voltage);
  ULP_INA219_PUBLISH(voltage);
  ULP_INA219_PUBLISH(voltage_max);
  ULP_INA219_PUBLISH(voltage_min);

  ESP_LOGD(TAG,
           "ulp info:\n"
           "  run dur: %.3f ms\n"
           "  slow clk: %u\n"
           "  state: %u\n"
           "  led cntr: %u",
           (float) ctx->run_duration / 1000.f, ctx->slow_clk_period, ctx->prg_state, ctx->led.counter);
}

void UlpIna219Sensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Bus index: %u", this->bus_index_);
  LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("  ", "Current", this->current_sensor_);
  LOG_SENSOR("  ", "Power", this->power_sensor_);
  LOG_SENSOR("  ", "Shunt Voltage", this->shunt_voltage_sensor_);
  LOG_SENSOR("  ", "Energy (Net)", this->energy_net_sensor_);
  LOG_SENSOR("  ", "Energy (In)", this->energy_in_sensor_);
  LOG_SENSOR("  ", "Energy (Out)", this->energy_out_sensor_);
  LOG_SENSOR("  ", "Charge (Net)", this->charge_net_sensor_);
  LOG_SENSOR("  ", "Charge (In)", this->charge_in_sensor_);
  LOG_SENSOR("  ", "Charge (Out)", this->charge_out_sensor_);
  LOG_SENSOR("  ", "Voltage (max)", this->voltage_max_sensor_);
  LOG_SENSOR("  ", "Voltage (min)", this->voltage_min_sensor_);
  LOG_SENSOR("  ", "Current (max)", this->current_max_sensor_);
  LOG_SENSOR("  ", "Current (min)", this->current_min_sensor_);
  LOG_SENSOR("  ", "Power (max)", this->power_max_sensor_);
  LOG_SENSOR("  ", "Power (min)", this->power_min_sensor_);

  LOG_UPDATE_INTERVAL(this);
}

}  // namespace ulp_ina219
}  // namespace esphome
