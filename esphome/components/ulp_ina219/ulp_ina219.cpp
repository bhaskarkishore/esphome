
#include <stdio.h>
#include "esp_sleep.h"
#include "ulp_main.h"
#include "ulp_lp_core.h"
#include "lp_core_i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/rtc.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "ulp_ina219.h"
#include <cmath>

#define LED_GREEN GPIO_NUM_2
#define BUS_A_MAX_VOLTAGE 5.0f
#define BUS_A_MAX_CURRENT 0.5f
#define BUS_A_ADDRESS 0x40
#define BUS_A_SHUNT_R 0.1126f
#define BUS_A_CURRENT_CLAMP_THRESHOLD 0.0005f

#define BUS_B_MAX_VOLTAGE 12.0f
#define BUS_B_MAX_CURRENT 0.5f
#define BUS_B_ADDRESS 0x41
#define BUS_B_SHUNT_R 0.1f
#define BUS_B_CURRENT_CLAMP_THRESHOLD 0.0005f

namespace esphome {
namespace ulp_ina219 {

extern const uint8_t lp_core_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t lp_core_main_bin_end[] asm("_binary_ulp_main_bin_end");

static void lp_core_init(void) {
  esp_err_t ret = ESP_OK;

  ulp_lp_core_cfg_t cfg = {.wakeup_source = ULP_LP_CORE_WAKEUP_SOURCE_LP_TIMER,
                           .lp_timer_sleep_duration_us = 177 * 1000};

  ret = ulp_lp_core_load_binary(lp_core_main_bin_start, (lp_core_main_bin_end - lp_core_main_bin_start));
  if (ret != ESP_OK) {
    printf("LP Core load failed\n");
    abort();
  }

  rtc_gpio_init(LED_GREEN);
  rtc_gpio_set_direction(LED_GREEN, RTC_GPIO_MODE_OUTPUT_ONLY);
  rtc_gpio_pulldown_dis(LED_GREEN);
  rtc_gpio_pullup_dis(LED_GREEN);

  ulp_slow_clk_period = rtc_clk_cal(RTC_CAL_RTC_MUX, 1000);

  // Set parameters
  ((uint8_t *) &ulp_cfg_bus_enabled)[0] = 1;
  ((float *) &ulp_cfg_bus_max_voltage)[0] = BUS_A_MAX_VOLTAGE;
  ((float *) &ulp_cfg_bus_max_current)[0] = BUS_A_MAX_CURRENT;
  ((uint8_t *) &ulp_cfg_bus_address)[0] = BUS_A_ADDRESS;
  ((float *) &ulp_cfg_bus_shunt_resistance)[0] = BUS_A_SHUNT_R;
  ((float *) &ulp_cfg_bus_current_clamp_threshold)[0] = BUS_A_CURRENT_CLAMP_THRESHOLD;
  ((float *) &ulp_cfg_bus_power_clamp_threshold)[0] = 0;

  ((uint8_t *) &ulp_cfg_bus_enabled)[1] = 1;
  ((float *) &ulp_cfg_bus_max_voltage)[1] = BUS_B_MAX_VOLTAGE;
  ((float *) &ulp_cfg_bus_max_current)[1] = BUS_B_MAX_CURRENT;
  ((uint8_t *) &ulp_cfg_bus_address)[1] = BUS_B_ADDRESS;
  ((float *) &ulp_cfg_bus_shunt_resistance)[1] = BUS_B_SHUNT_R;
  ((float *) &ulp_cfg_bus_current_clamp_threshold)[1] = BUS_B_CURRENT_CLAMP_THRESHOLD;
  ((float *) &ulp_cfg_bus_power_clamp_threshold)[1] = 0;

  ((uint8_t *) &ulp_bus_reset)[0] = 1;
  ((uint8_t *) &ulp_bus_reset)[1] = 1;

  ret = ulp_lp_core_run(&cfg);
  if (ret != ESP_OK) {
    printf("LP Core run failed\n");
    abort();
  }

  ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());
  printf("LP core loaded with firmware successfully\n");
}

static void lp_i2c_init(void) {
  esp_err_t ret = ESP_OK;

  /* Initialize LP I2C with default configuration */
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
              .clk_speed_hz = 400000,  // 100 kHz (standard I2C speed)
          },
      .i2c_src_clk = LP_I2C_SCLK_DEFAULT,  // Default LP I2C source clock
  };
  ret = lp_core_i2c_master_init(LP_I2C_NUM_0, &i2c_cfg);
  if (ret != ESP_OK) {
    printf("LP I2C init failed\n");
    abort();
  }

  printf("LP I2C initialized successfully\n");
}

static const char *const TAG = "ulp_219";

void UlpIna219::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Custom Sensor...");
  lp_i2c_init();
  lp_core_init();

  // Initialize any hardware or configurations here
  // For ESP32-C6/H2 specific initialization if needed

#if defined(USE_ESP32_VARIANT_ESP32C6) || defined(USE_ESP32_VARIANT_ESP32H2)
  ESP_LOGD(TAG, "Running on supported ESP32 variant");
#else
  ESP_LOGE(TAG, "Unsupported ESP32 variant - component requires ESP32-C6 or ESP32-H2");
  this->mark_failed();
  return;
#endif
}

void UlpIna219::update() {
  ESP_LOGD(TAG, "Updating sensor readings...");

  ESP_LOGD(TAG, "Bus Errors 0:%d, 1:%d", ((esp_err_t *) &ulp_bus_error_code)[0],
           ((esp_err_t *) &ulp_bus_error_code)[1]);
  if (this->voltage_sensor_ != nullptr) {
    float voltage = ((float *) &ulp_bus_voltage)[0];
    this->voltage_sensor_->publish_state(voltage);
    ESP_LOGD(TAG, "Voltage: %.2f V", voltage);
  }

  if (this->current_sensor_ != nullptr) {
    float current = ((float *) &ulp_bus_current)[0];
    this->current_sensor_->publish_state(current);
    ESP_LOGD(TAG, "Current: %.3f A", current);
  }

  if (this->power_sensor_ != nullptr) {
    float power = ((float *) &ulp_bus_power)[0];
    this->power_sensor_->publish_state(power);
    ESP_LOGD(TAG, "Power: %.3f W", power);
  }

  if (this->shunt_voltage_sensor_ != nullptr) {
    float shunt_voltage = ((float *) &ulp_bus_shunt_voltage)[0];
    this->shunt_voltage_sensor_->publish_state(shunt_voltage);
    ESP_LOGD(TAG, "Shunt Voltage: %.3f V", shunt_voltage);
  }

  if (this->energy_sensor_ != nullptr) {
    float energy = ((float *) &ulp_bus_energy)[0];
    this->energy_sensor_->publish_state(energy);
    ESP_LOGD(TAG, "Energy: %.3f Wh", energy);
  }

  if (this->charge_sensor_ != nullptr) {
    float charge = ((float *) &ulp_bus_charge)[0];
    this->charge_sensor_->publish_state(charge);
    ESP_LOGD(TAG, "charge: %.3f Ah", charge);
  }

  if (this->duration_sensor_ != nullptr) {
    float duration = (float) ulp_run_duration / 1000;
    this->duration_sensor_->publish_state(duration);
    ESP_LOGD(TAG, "Ulp Runtime: %.3f ms", duration);
  }
}

void UlpIna219::dump_config() {
  ESP_LOGCONFIG(TAG, "Custom Sensor:");
  LOG_UPDATE_INTERVAL(this);

  if (this->voltage_sensor_ != nullptr) {
    LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  }
  if (this->current_sensor_ != nullptr) {
    LOG_SENSOR("  ", "Current", this->current_sensor_);
  }
  if (this->duration_sensor_ != nullptr) {
    LOG_SENSOR("  ", "Duration", this->duration_sensor_);
  }

  if (this->is_failed()) {
    ESP_LOGE(TAG, "Component setup failed!");
  }
}

}  // namespace ulp_ina219
}  // namespace esphome