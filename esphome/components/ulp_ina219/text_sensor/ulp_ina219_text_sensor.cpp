#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/ulp_ina219/text_sensor/ulp_ina219_text_sensor.h"

namespace esphome {
namespace ulp_ina219 {

static const char *const TAG = "ulp_ina219.text_sensor";

static const std::string TEXT_NOT_SET = "Not Set";
static const std::string TEXT_SET = "Set";
static const std::string TEXT_FIRED = "Fired";

void UlpIna219TextSensor::setup() { this->last_status_ = ALARM_NOT_SET; }

void UlpIna219TextSensor::update() {
  volatile ulp_ina219_context_t *ctx = UlpIna219::get_ulp_context();
  volatile alarm_t *t = &ctx->buses[this->bus_idx_].alarms[this->alarm_idx_];

  if (t->status == ALARM_NOT_SET || t->status == this->last_status_) {
    return;
  } else {
    this->last_status_ = t->status;
    if (t->status == ALARM_SET) {
      this->publish_state(TEXT_SET);
    } else if (t->status == ALARM_FIRED) {
      this->publish_state(TEXT_FIRED);
    }
  }
}

void UlpIna219TextSensor::dump_config() {
  volatile ulp_ina219_context_t *ctx = UlpIna219::get_ulp_context();
  volatile alarm_t *t = &ctx->buses[this->bus_idx_].alarms[this->alarm_idx_];
  ESP_LOGCONFIG(TAG,
                "UlpIna219 Alarm:\n"
                "  bus index: %u\n"
                "  alarm index: %u\n"
                "  mode: %u\n"
                "  above: %f\n"
                "  below: %f\n"
                "  threshold: %f\n",
                this->bus_idx_, this->alarm_idx_, t->mode, t->condition.range.above, t->condition.range.below,
                t->condition.delta.threshold);
  LOG_TEXT_SENSOR("  ", "Alarm", this);
}

}  // namespace ulp_ina219
}  // namespace esphome
