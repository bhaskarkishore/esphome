#include "adc_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#ifdef USE_ESP8266
#ifdef USE_ADC_SENSOR_VCC
#include <Esp.h>
ADC_MODE(ADC_VCC)
#else
#include <Arduino.h>
#endif
#endif

namespace esphome {
namespace adc {

static const char *const TAG = "adc";

/*---------------------------------------------------------------
        ADC General Macros
---------------------------------------------------------------*/
//ADC1 Channels
#if CONFIG_IDF_TARGET_ESP32
#define ADC1_CHAN0          ADC_CHANNEL_4
#define ADC1_CHAN1          ADC_CHANNEL_5
#else
#define ADC1_CHAN0          ADC_CHANNEL_0    // GPIO0
#define ADC1_CHAN1          ADC_CHANNEL_1    // GPIO1
#define ADC1_CHAN2          ADC_CHANNEL_2    // GPIO2
#define ADC1_CHAN3          ADC_CHANNEL_3    // GPIO3
#define ADC1_CHAN4          ADC_CHANNEL_4    // GPIO4
#define ADC1_CHAN5          ADC_CHANNEL_5    // GPIO5
#define ADC1_CHAN6          ADC_CHANNEL_6    // GPIO6
#endif

#define ADC_ATTEN           ADC_ATTEN_DB_12   // The input voltage of ADC will be attenuated extending the range of measurement by about 12 dB.


static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void adc_calibration_deinit(adc_cali_handle_t handle);

void ADCSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ADC '%s'...", this->get_name().c_str());

  // Create new oneshot ADC unit
  adc_oneshot_unit_handle_t adc1_handle;
  adc_oneshot_unit_init_cfg_t init_config1 = {
      .unit_id = ADC_UNIT_1,                    // Selects the ADC
      // .clk_src = 0,                             // Selects the source clock of the ADC. If set to 0, the driver will fall back to using a default clock source
      .ulp_mode = ADC_ULP_MODE_DISABLE,         // Sets if the ADC will be working under ULP mode.
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
  
  // Set ADC parameters
  adc_oneshot_chan_cfg_t config = {
      .atten = ADC_ATTEN,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
  };

  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, this->channel1_, &config));
  
  ESP_LOGCONFIG(TAG, "ADC '%s' setup finished!", this->get_name().c_str());
}

void ADCSensor::dump_config() {
  LOG_SENSOR("", "ADC Sensor", this);
#if defined(USE_ESP8266) || defined(USE_LIBRETINY)
#ifdef USE_ADC_SENSOR_VCC
  ESP_LOGCONFIG(TAG, "  Pin: VCC");
#else
  LOG_PIN("  Pin: ", this->pin_);
#endif
#endif  // USE_ESP8266 || USE_LIBRETINY

#ifdef USE_ESP32
  LOG_PIN("  Pin: ", this->pin_);
  if (this->autorange_) {
    ESP_LOGCONFIG(TAG, "  Attenuation: auto");
  } /*else {
    switch (this->attenuation_) {
      case ADC_ATTEN_DB_0:
        ESP_LOGCONFIG(TAG, "  Attenuation: 0db");
        break;
      case ADC_ATTEN_DB_2_5:
        ESP_LOGCONFIG(TAG, "  Attenuation: 2.5db");
        break;
      case ADC_ATTEN_DB_6:
        ESP_LOGCONFIG(TAG, "  Attenuation: 6db");
        break;
      case ADC_ATTEN_DB_12_COMPAT:
        ESP_LOGCONFIG(TAG, "  Attenuation: 12db");
        break;
      default:  // This is to satisfy the unused ADC_ATTEN_MAX
        break;
    }
  }
  */
#endif  // USE_ESP32

  ESP_LOGCONFIG(TAG, "  Samples: %i", this->sample_count_);
  LOG_UPDATE_INTERVAL(this);
}

float ADCSensor::get_setup_priority() const {
   return setup_priority::DATA;
}

void ADCSensor::update() {
  float value_v = this->sample();
  ESP_LOGV(TAG, "'%s': Got voltage=%.4fV", this->get_name().c_str(), value_v);
  this->publish_state(value_v);
}

void ADCSensor::set_sample_count(uint8_t sample_count) {
  if (sample_count != 0) {
    this->sample_count_ = sample_count;
  }
}

#ifdef USE_ESP32
float ADCSensor::sample()
{
  int sum = 0;
  int voltage = 0;
  
  // Read ADC raw values averaging depending on sample count
  for (uint8_t sample = 0; sample < this->sample_count_; sample++) {
    int raw = -1;
    adc_oneshot_read(this->handle1_, this->channel1_, &raw);
    if (raw == -1) {
      return NAN;
    }
    sum += raw;
  }
  sum = (sum + (this->sample_count_ >> 1)) / this->sample_count_;  // NOLINT(clang-analyzer-core.DivideZero)
  
  ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, this->channel1_, sum);

  // Calibrate the ADC channel  
  bool is_calibrated = adc_calibration_init(ADC_UNIT_1, this->channel1_, ADC_ATTEN, &this->cali_handle_);

  if (is_calibrated) {
      // Convert the ADC raw result into calibrated result
      ESP_ERROR_CHECK(adc_cali_raw_to_voltage(this->cali_handle_, sum, &voltage));
      ESP_LOGI(TAG, "ADC%d Channel[%d] Calibrated Voltage: %d mV", ADC_UNIT_1 + 1, this->channel1_, voltage);
  }
  
  // Free cali resources
  if (is_calibrated) {
      adc_calibration_deinit(this->cali_handle_);
  }
  
  float result = (float)(voltage) / 1000;

  return result;
}

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        // ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        // ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success, ADC%d Channel[%d]", unit + 1, channel);
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGI(TAG, "eFuse not burnt, skip software calibration, ADC%d Channel[%d]", unit + 1, channel);
    } else {
        ESP_LOGI(TAG, "Invalid arg or no memory, ADC%d Channel[%d]", unit + 1, channel);
    }

    return calibrated;
}

static void adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    // ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    // ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}
#endif  // USE_ESP32

}  // namespace adc
}  // namespace esphome
