#include <stdio.h>
#include "esp_sleep.h"
#include "ulp_main.h"
#include "ulp_lp_core.h"
#include "lp_core_i2c.h"
#include "soc/rtc.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "ulp_ina219.h"
#include <cmath>
#include "ulp/types.h"

#define LED_GREEN GPIO_NUM_2

namespace esphome {
namespace ulp_ina219 {

RTC_DATA_ATTR double total_charge[MAX_BUS] = {0.f, 0.f};
RTC_DATA_ATTR double total_energy[MAX_BUS] = {0.f, 0.f};

extern const uint8_t lp_core_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t lp_core_main_bin_end[] asm("_binary_ulp_main_bin_end");

static const char *const TAG = "ulp_219";

void UlpIna219::setup() {
  ESP_LOGCONFIG(TAG, "Running setup...");

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  volatile state_t *state = ((state_t *) &ulp_state);

  if (state->prg_state == 0 || cause == ESP_SLEEP_WAKEUP_UNDEFINED) {
    ulp_lp_core_stop();

    esp_err_t ret = this->lp_i2c_init_();
    if (ret != ESP_OK) {
      this->mark_failed("Unable to initialize ulp i2c");
      return;
    }
    ret = this->lp_core_init_();
    if (ret != ESP_OK) {
      this->mark_failed("Unable to initialize ulp core");
      return;
    }
    ESP_LOGCONFIG(TAG, "Ulp started");
  } else {
    // Recalibrate the slow clock period
    state->slow_clk_period = rtc_clk_cal(RTC_CAL_RTC_MUX, 1000);
    esp_sleep_enable_ulp_wakeup();
    ESP_LOGCONFIG(TAG, "Ulp already running");
  }
}

esp_err_t UlpIna219::lp_core_init_(void) {
  esp_err_t ret = ESP_OK;

  ulp_lp_core_cfg_t cfg = {.wakeup_source = ULP_LP_CORE_WAKEUP_SOURCE_LP_TIMER,
                           .lp_timer_sleep_duration_us = this->sleep_duration_ * 1000};

  ret = ulp_lp_core_load_binary(lp_core_main_bin_start, (lp_core_main_bin_end - lp_core_main_bin_start));
  if (ret != ESP_OK) {
    ESP_LOGD(TAG, "Ulp binary load failed");
    return ret;
  }

  state_t *state = (state_t *) &ulp_state;
  general_cfg_t *gcfg = (general_cfg_t *) &ulp_cfg;

  if (this->led_pin_ != GPIO_NUM_NC && this->led_interval_ > 0) {
    rtc_gpio_init(this->led_pin_);
    rtc_gpio_set_direction(this->led_pin_, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_pulldown_dis(this->led_pin_);
    rtc_gpio_pullup_dis(this->led_pin_);
    gcfg->led_interval = this->led_interval_;
    gcfg->led_gpio = this->led_pin_;
  } else {
    gcfg->led_interval = 0;
    gcfg->led_gpio = -1;
  }

  state->slow_clk_period = rtc_clk_cal(RTC_CAL_RTC_MUX, 1000);

  for (uint8_t i = 0; i < MAX_BUS; ++i) {
    bus_cfg_t *c = &((bus_cfg_t *) &ulp_bus_cfg)[i];
    if (this->bus_enabled_[i]) {
      ESP_LOGD(TAG, "Bus %c enabled", i == 0 ? 'A' : 'B');
      c->max_system_voltage = this->max_system_voltage_[i];
      c->max_system_current = this->max_system_current_[i];
      c->address = this->address_[i];
      c->shunt_resistance = this->shunt_resistance_[i];
      c->current_accum_threshold = this->current_accum_threshold_[i];
      c->power_accum_threshold = this->power_accum_threshold_[i];
      c->calibration_register = this->calibration_register_[i];
      c->reset = 1;

    } else {
      c->address = 0x0;
      ESP_LOGD(TAG, "Bus %c disabled", i == 0 ? 'A' : 'B');
    }
  }

  ret = ulp_lp_core_run(&cfg);
  if (ret != ESP_OK) {
    ESP_LOGD(TAG, "Ulp start failed");
    return ret;
  }

  ret = esp_sleep_enable_ulp_wakeup();

  if (ret != ESP_OK) {
    ESP_LOGD(TAG, "Ulp wake src set failed");
    return ret;
  }
  ESP_LOGD(TAG, "Ulp start complete");
  return ret;
}

esp_err_t UlpIna219::lp_i2c_init_(void) {
  const lp_core_i2c_cfg_t i2c_cfg = {
      .i2c_pin_cfg =
          {
              .sda_io_num = this->lp_sda_pin_,
              .scl_io_num = this->lp_scl_pin_,
              .sda_pullup_en = this->lp_sda_pullup_en_,
              .scl_pullup_en = this->lp_scl_pullup_en_,
          },
      .i2c_timing_cfg =
          {
              .clk_speed_hz = 400000,  // 400 kHz
          },
      .i2c_src_clk = LP_I2C_SCLK_DEFAULT,  // Default LP I2C source clock
  };
  return lp_core_i2c_master_init(LP_I2C_NUM_0, &i2c_cfg);
}

void UlpIna219::set_total_charge(uint8_t bus_idx, double charge) {
  if (bus_idx < MAX_BUS) {
    total_charge[bus_idx] = charge;
  }
}

void UlpIna219::set_total_energy(uint8_t bus_idx, double energy) {
  if (bus_idx < MAX_BUS) {
    total_energy[bus_idx] = energy;
  }
}

void UlpIna219::update() {
  volatile state_t *state = ((state_t *) &ulp_state);
  for (uint8_t i = 0; i < MAX_BUS; ++i) {
    if (!this->bus_enabled_[i])
      continue;

    bus_values_t *b = &((bus_values_t *) &ulp_bus)[i];
    bus_cfg_t *c = &((bus_cfg_t *) &ulp_bus_cfg)[i];

    char bus = i == 0 ? 'a' : 'b';

    ESP_LOGD(TAG, "Bus %c:", bus);
    ESP_LOGD(TAG, "Calibration register: %u", b->calibration_register);
    ESP_LOGD(TAG, "Current LSB: %u", b->current_lsb);

    esp_err_t bus_error_code = b->error_code;
    if (bus_error_code != ESP_OK) {
      ESP_LOGD(TAG, "Bus has errors, code: 0x%X", bus_error_code);
      this->mark_failed("error reading device");
      continue;
    }

    if (this->voltage_sensor_[i] != nullptr) {
      this->voltage_sensor_[i]->publish_state(b->voltage);
    }

    if (this->current_sensor_[i] != nullptr) {
      this->current_sensor_[i]->publish_state(b->current);
    }

    if (this->power_sensor_[i] != nullptr) {
      this->power_sensor_[i]->publish_state(b->power);
    }

    if (this->shunt_voltage_sensor_[i] != nullptr) {
      this->shunt_voltage_sensor_[i]->publish_state(b->shunt_voltage);
    }

    if (this->energy_sensor_[i] != nullptr) {
      total_energy[i] = total_energy[i] + b->energy;
      this->energy_sensor_[i]->publish_state(total_energy[i]);
    }

    if (this->charge_sensor_[i] != nullptr) {
      total_charge[i] = total_charge[i] + b->charge;
      this->charge_sensor_[i]->publish_state(total_charge[i]);
    }

    if (this->voltage_max_sensor_[i] != nullptr) {
      this->voltage_max_sensor_[i]->publish_state(b->voltage_max);
    }

    if (this->voltage_min_sensor_[i] != nullptr) {
      this->voltage_min_sensor_[i]->publish_state(b->voltage_min);
    }

    if (this->current_max_sensor_[i] != nullptr) {
      this->current_max_sensor_[i]->publish_state(b->current_max);
    }

    if (this->current_min_sensor_[i] != nullptr) {
      this->current_min_sensor_[i]->publish_state(b->current_min);
    }

    if (this->power_max_sensor_[i] != nullptr) {
      this->power_max_sensor_[i]->publish_state(b->power_max);
    }

    if (this->power_min_sensor_[i] != nullptr) {
      this->power_min_sensor_[i]->publish_state(b->power_min);
    }

    c->reset = 1;
  }

  ESP_LOGD(TAG, "Ulp prg run duration: %.3f", (double) state->run_duration / 1000.f);
}

void UlpIna219::dump_config() {
  ESP_LOGCONFIG(TAG, "ULP INA219:");

  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }

  ESP_LOGCONFIG(TAG, "Sleep Duration: %d", this->sleep_duration_);
  ESP_LOGCONFIG(TAG, "lp_i2c:");
  ESP_LOGCONFIG(TAG, "  SDA Pin: %d", this->lp_sda_pin_);
  ESP_LOGCONFIG(TAG, "  SDA Pull Enabled: %d", this->lp_sda_pullup_en_);
  ESP_LOGCONFIG(TAG, "  SCL Pin: %d", this->lp_scl_pin_);
  ESP_LOGCONFIG(TAG, "  SCL Pull Enabled: %d", this->lp_scl_pullup_en_);

  if (this->led_pin_ != GPIO_NUM_NC) {
    ESP_LOGCONFIG(TAG, "led:");
    ESP_LOGCONFIG(TAG, "  pin: %d", this->led_pin_);
    ESP_LOGCONFIG(TAG, "  interval: %d", this->led_interval_);
  }

  for (uint8_t i = 0; i < MAX_BUS; ++i) {
    if (this->bus_enabled_[i]) {
      ESP_LOGCONFIG(TAG, "bus_%c:", i == 0 ? 'a' : 'b');
      ESP_LOGCONFIG(TAG, "  Address: %X", this->address_[i]);
      ESP_LOGCONFIG(TAG, "  Shunt Resistance: %f", this->shunt_resistance_[i]);
      ESP_LOGCONFIG(TAG, "  Max Voltage: %f", this->max_system_voltage_[i]);
      ESP_LOGCONFIG(TAG, "  Max Current: %f", this->max_system_current_[i]);
      ESP_LOGCONFIG(TAG, "  Current Accumulation Threshold: %f", this->current_accum_threshold_[i]);
      ESP_LOGCONFIG(TAG, "  Power Accumulation Threshold: %f", this->power_accum_threshold_[i]);
      ESP_LOGCONFIG(TAG, "  Calibration Register: %f", this->calibration_register_[i]);
      LOG_SENSOR("  ", "Voltage", this->voltage_sensor_[i]);
      LOG_SENSOR("  ", "Current", this->current_sensor_[i]);
      LOG_SENSOR("  ", "Power", this->power_sensor_[i]);
      LOG_SENSOR("  ", "Shunt Voltage", this->shunt_voltage_sensor_[i]);
      LOG_SENSOR("  ", "Energy", this->energy_sensor_[i]);
      LOG_SENSOR("  ", "Charge", this->charge_sensor_[i]);
      LOG_SENSOR("  ", "Voltage (max)", this->voltage_max_sensor_[i]);
      LOG_SENSOR("  ", "Voltage (min)", this->voltage_min_sensor_[i]);
      LOG_SENSOR("  ", "Current (max)", this->current_max_sensor_[i]);
      LOG_SENSOR("  ", "Current (min)", this->current_min_sensor_[i]);
      LOG_SENSOR("  ", "Power (max)", this->power_max_sensor_[i]);
      LOG_SENSOR("  ", "Power (min)", this->power_min_sensor_[i]);
    }
  }

  LOG_UPDATE_INTERVAL(this);
}

}  // namespace ulp_ina219
}  // namespace esphome