
#include "ulp_lp_core_gpio.h"
#include "soc/lp_timer_reg.h"
#include "soc/rtc.h"
#include "ina219.h"

// NOTE: All global types are uint32_t, application must handle casting

// Cross processor state mangement
#define PRG_STATE_NONE 0
#define PRG_STATE_RUNNING 1
#define PRG_STATE_SLEEPING 2

#define SAMPLING_DELAY_WAIT 75 * 1000
#define MAX_BUS 2

// State variables
uint32_t slow_clk_period;
uint64_t run_duration;
volatile uint8_t prg_state = PRG_STATE_NONE;
uint8_t led_interval_counter = 0;

// ina219 configuration variables
uint8_t cfg_bus_address[MAX_BUS] = {0, 0};
float cfg_bus_max_voltage[MAX_BUS] = {0.f, 0.f};
float cfg_bus_max_current[MAX_BUS] = {0.f, 0.f};
float cfg_bus_shunt_resistance[MAX_BUS] = {0.f, 0.f};
float cfg_bus_current_accum_threshold[MAX_BUS] = {0.f, 0.f};
float cfg_bus_power_accum_threshold[MAX_BUS] = {0.f, 0.f};
uint32_t cfg_bus_calibration_register[MAX_BUS] = {0, 0};

// Other configuration variables
uint8_t cfg_led_interval = 10;
uint8_t cfg_led_gpio = LP_IO_NUM_2;

// Output variables
volatile esp_err_t bus_error_code[MAX_BUS] = {ESP_OK, ESP_OK};

volatile uint8_t bus_reset[MAX_BUS] = {1, 1};
volatile uint64_t bus_last_sample_time[MAX_BUS] = {0, 0};

volatile float bus_charge[MAX_BUS] = {0.f, 0.f};  // in Ah
volatile float bus_energy[MAX_BUS] = {0.f, 0.f};  // in Wh

volatile uint32_t bus_calibration_register[MAX_BUS] = {0, 0};
volatile uint32_t bus_current_lsb[MAX_BUS] = {0, 0};

volatile float bus_voltage[MAX_BUS] = {0.f, 0.f};
volatile float bus_current[MAX_BUS] = {0.f, 0.f};
volatile float bus_power[MAX_BUS] = {0.f, 0.f};
volatile float bus_shunt_voltage[MAX_BUS] = {0.f, 0.f};

volatile float bus_voltage_min[MAX_BUS] = {0.f, 0.f};
volatile float bus_voltage_max[MAX_BUS] = {0.f, 0.f};

volatile float bus_current_min[MAX_BUS] = {0.f, 0.f};
volatile float bus_current_max[MAX_BUS] = {0.f, 0.f};

volatile float bus_power_min[MAX_BUS] = {0.f, 0.f};
volatile float bus_power_max[MAX_BUS] = {0.f, 0.f};

static float clamp(float value, float threshold) {
  float v = value < 0.0f ? -value : value;
  if (v <= threshold) {
    return 0.f;
  }
  return value;
}

static void min_max(float value, volatile float *min, volatile float *max, uint32_t reset) {
  if (reset) {
    *min = *max = value;
  } else {
    if (*max < value) {
      *max = value;
    }
    if (*min > value) {
      *min = value;
    }
  }
}

static uint64_t lp_core_get_rtc_ticks(void) {
  uint32_t ticks_low, ticks_high;
  REG_WRITE(LP_TIMER_UPDATE_REG, LP_TIMER_MAIN_TIMER_UPDATE);
  ticks_low = REG_READ(LP_TIMER_MAIN_BUF0_LOW_REG);
  ticks_high = REG_READ(LP_TIMER_MAIN_BUF0_HIGH_REG);
  return ((uint64_t) ticks_high << 32) | ticks_low;
}

static uint64_t lp_core_rtc_ticks_to_us(uint64_t ticks, uint64_t period) {
  return (ticks * period) >> RTC_CLK_CAL_FRACT;
}

static void accumulate(uint32_t reset, volatile float *acc_charge, float previous_current, float sampled_current,
                       volatile float *acc_energy, float previous_power, float sampled_power,
                       float current_clamp_threshold, float power_clamp_threshold,
                       volatile uint64_t *last_sample_time) {
  sampled_current = clamp(sampled_current, current_clamp_threshold);
  sampled_power = clamp(sampled_power, power_clamp_threshold);
  uint64_t current_time = lp_core_rtc_ticks_to_us(lp_core_get_rtc_ticks(), slow_clk_period);
  float time_delta_hours = (current_time - *last_sample_time) / 3600000000.0f;

  float charge = 0.f, energy = 0.f;
  if (*last_sample_time != 0 && current_time > *last_sample_time) {
    charge = ((sampled_current + previous_current) / 2.0f) * time_delta_hours;
    energy = ((sampled_power + previous_power) / 2.0f) * time_delta_hours;
    *acc_energy += energy;
    *acc_charge += charge;
  }

  if (reset) {
    // This ensures that the present sample is not lost
    // when reset is called.
    *acc_energy = energy;
    *acc_charge = charge;
  }

  *last_sample_time = current_time;
}

static void process() {
  // The adc is set to 128 samples which takes around 68 ms
  // to finish according as per the datasheet. We wait a few
  // ms more.
  ulp_lp_core_delay_us(SAMPLING_DELAY_WAIT);

  float previous_current = 0.f, previous_power = 0.f;
  for (uint8_t i = 0; i < MAX_BUS; ++i) {
    if (cfg_bus_address[i] > 0 && bus_error_code[i] == ESP_OK) {
      previous_current = bus_current[i];
      previous_power = bus_power[i];

      // Sample sensor
      ina219_bus_voltage(cfg_bus_address[i], &bus_voltage[i]);
      ina219_power(cfg_bus_address[i], &bus_power[i], bus_current_lsb[i]);
      ina219_shunt_voltage(cfg_bus_address[i], &bus_shunt_voltage[i]);
      ina219_current(cfg_bus_address[i], &bus_current[i], bus_current_lsb[i]);

      // Compute min max
      min_max(bus_current[i], &bus_current_min[i], &bus_current_max[i], bus_reset[i]);
      min_max(bus_power[i], &bus_power_min[i], &bus_power_max[i], bus_reset[i]);
      min_max(bus_voltage[i], &bus_voltage_min[i], &bus_voltage_max[i], bus_reset[i]);

      // Accumulate current and energy
      accumulate(bus_reset[i], &bus_charge[i], previous_current, bus_current[i], &bus_energy[i], previous_power,
                 bus_power[i], cfg_bus_current_accum_threshold[i], cfg_bus_power_accum_threshold[i],
                 &bus_last_sample_time[i]);

      // Clear reset if set
      bus_reset[i] = 0;

      // Power down
      ina219_power_down(cfg_bus_address[i]);
    }
  }
}

static void init() {
  esp_err_t ret = ESP_OK;
  uint32_t calibration_register = 0;
  uint32_t current_lsb = 0;

  for (uint8_t i = 0; i < MAX_BUS; ++i) {
    bus_error_code[i] = ESP_OK;
    if (cfg_bus_address[i] > 0) {
      ret = ina219_init(cfg_bus_address[i], cfg_bus_max_voltage[i], cfg_bus_shunt_resistance[i], cfg_bus_max_current[i],
                        cfg_bus_calibration_register[i], &calibration_register, &current_lsb);
      if (ret == ESP_OK) {
        bus_current_lsb[i] = current_lsb;
        bus_calibration_register[i] = calibration_register;
      } else {
        bus_error_code[i] = ret;
      }
    }
  }
}

int main(void) {
  prg_state = PRG_STATE_RUNNING;
  uint64_t current = lp_core_get_rtc_ticks();

  if (led_interval_counter >= cfg_led_interval) {
    ulp_lp_core_gpio_set_level(cfg_led_gpio, 1);
    led_interval_counter = 0;
  }

  init();
  process();

  led_interval_counter++;
  ulp_lp_core_gpio_set_level(cfg_led_gpio, 0);

  run_duration = lp_core_rtc_ticks_to_us(lp_core_get_rtc_ticks() - current, slow_clk_period);
  prg_state = PRG_STATE_SLEEPING;

  return 0;
}
