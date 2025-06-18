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

#define LED_GREEN GPIO_NUM_2

namespace esphome {
namespace ulp_ina219 {

extern const uint8_t lp_core_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t lp_core_main_bin_end[] asm("_binary_ulp_main_bin_end");

static const char *const TAG = "ulp_219";

void UlpIna219::setup() {
  ESP_LOGCONFIG(TAG, "Running setup...");

  if (ulp_prg_state == 0) {
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
    ESP_LOGCONFIG(TAG, "Ulp already running");
  }
}

esp_err_t UlpIna219::lp_core_init_(void) {
  esp_err_t ret = ESP_OK;

  ulp_lp_core_cfg_t cfg = {.wakeup_source = ULP_LP_CORE_WAKEUP_SOURCE_LP_TIMER,
                           .lp_timer_sleep_duration_us = 177 * 1000};

  ret = ulp_lp_core_load_binary(lp_core_main_bin_start, (lp_core_main_bin_end - lp_core_main_bin_start));
  if (ret != ESP_OK) {
    ESP_LOGD(TAG, "Ulp binary load failed");
    return ret;
  }

  rtc_gpio_init(LED_GREEN);
  rtc_gpio_set_direction(LED_GREEN, RTC_GPIO_MODE_OUTPUT_ONLY);
  rtc_gpio_pulldown_dis(LED_GREEN);
  rtc_gpio_pullup_dis(LED_GREEN);

  ulp_slow_clk_period = rtc_clk_cal(RTC_CAL_RTC_MUX, 1000);

  for (uint8_t i = 0; i < MAX_BUSES; ++i) {
    if (this->bus_enabled_[i]) {
      ESP_LOGD(TAG, "Bus %c enabled", i == 0 ? 'A' : 'B');
      ((float *) &ulp_cfg_bus_max_voltage)[i] = this->max_system_voltage_[i];
      ((float *) &ulp_cfg_bus_max_current)[i] = this->max_system_current_[i];
      ((uint8_t *) &ulp_cfg_bus_address)[i] = this->address_[i];
      ((float *) &ulp_cfg_bus_shunt_resistance)[i] = this->shunt_resistance_[i];
      ((float *) &ulp_cfg_bus_current_accum_threshold)[i] = this->current_accum_threshold_[i];
      ((float *) &ulp_cfg_bus_power_accum_threshold)[i] = this->power_accum_threshold_[i];
      ((uint8_t *) &ulp_bus_reset)[i] = 1;
    } else {
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
              .sda_io_num = GPIO_NUM_6,  // Fixed SDA pin (see datasheet)
              .scl_io_num = GPIO_NUM_7,  // Fixed SCL pin (see datasheet)
              .sda_pullup_en = true,     // Enable internal pullup for SDA
              .scl_pullup_en = true,     // Enable internal pullup for SCL
          },
      .i2c_timing_cfg =
          {
              .clk_speed_hz = 400000,  // 400 kHz
          },
      .i2c_src_clk = LP_I2C_SCLK_DEFAULT,  // Default LP I2C source clock
  };
  return lp_core_i2c_master_init(LP_I2C_NUM_0, &i2c_cfg);
}

void UlpIna219::update() {
  for (uint8_t i = 0; i < MAX_BUSES; ++i) {
    esp_err_t bus_error_code = ((esp_err_t *) &ulp_bus_error_code)[i];
    if (bus_error_code != ESP_OK) {
      ESP_LOGD(TAG, "Bus %c has errors, code: 0x%X", i == 0 ? 'A' : 'B', bus_error_code);
      this->mark_failed("error reading device");
      continue;
    }

    if (!this->bus_enabled_[i])
      continue;

    if (this->voltage_sensor_[i] != nullptr) {
      float voltage = ((float *) &ulp_bus_voltage)[i];
      this->voltage_sensor_[i]->publish_state(voltage);
    }

    if (this->current_sensor_[i] != nullptr) {
      float current = ((float *) &ulp_bus_current)[i];
      this->current_sensor_[i]->publish_state(current);
    }

    if (this->power_sensor_[i] != nullptr) {
      float power = ((float *) &ulp_bus_power)[i];
      this->power_sensor_[i]->publish_state(power);
    }

    if (this->shunt_voltage_sensor_[i] != nullptr) {
      float shunt_voltage = ((float *) &ulp_bus_shunt_voltage)[i];
      this->shunt_voltage_sensor_[i]->publish_state(shunt_voltage);
    }

    if (this->energy_sensor_[i] != nullptr) {
      float energy = ((float *) &ulp_bus_energy)[i];
      this->energy_sensor_[i]->publish_state(energy);
    }

    if (this->charge_sensor_[i] != nullptr) {
      float charge = ((float *) &ulp_bus_charge)[i];
      this->charge_sensor_[i]->publish_state(charge);
    }

    if (this->voltage_max_sensor_[i] != nullptr) {
      float vmax = ((float *) &ulp_bus_voltage_max)[i];
      this->voltage_max_sensor_[i]->publish_state(vmax);
    }

    if (this->voltage_min_sensor_[i] != nullptr) {
      float vmin = ((float *) &ulp_bus_voltage_min)[i];
      this->voltage_min_sensor_[i]->publish_state(vmin);
    }

    if (this->current_max_sensor_[i] != nullptr) {
      float cmax = ((float *) &ulp_bus_current_max)[i];
      this->current_max_sensor_[i]->publish_state(cmax);
    }

    if (this->current_min_sensor_[i] != nullptr) {
      float cmin = ((float *) &ulp_bus_current_min)[i];
      this->current_min_sensor_[i]->publish_state(cmin);
    }

    if (this->power_max_sensor_[i] != nullptr) {
      float cmax = ((float *) &ulp_bus_power_max)[i];
      this->power_max_sensor_[i]->publish_state(cmax);
    }

    if (this->power_min_sensor_[i] != nullptr) {
      float cmin = ((float *) &ulp_bus_power_min)[i];
      this->power_min_sensor_[i]->publish_state(cmin);
    }
  }

  if (this->ulp_run_duration_sensor_ != nullptr) {
    float duration = (float) ulp_run_duration / 1000;
    this->ulp_run_duration_sensor_->publish_state(duration);
  }
}

void UlpIna219::dump_config() {
  ESP_LOGCONFIG(TAG, "Sensor Config:");
  LOG_UPDATE_INTERVAL(this);

  // if (this->voltage_sensor_ != nullptr) {
  //   LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  // }
  // if (this->current_sensor_ != nullptr) {
  //   LOG_SENSOR("  ", "Current", this->current_sensor_);
  // }
  if (this->ulp_run_duration_sensor_ != nullptr) {
    LOG_SENSOR("  ", "Duration", this->ulp_run_duration_sensor_);
  }

  if (this->is_failed()) {
    ESP_LOGE(TAG, "Component setup failed!");
  }
}

}  // namespace ulp_ina219
}  // namespace esphome