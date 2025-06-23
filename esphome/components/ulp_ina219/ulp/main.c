
#include "ulp_lp_core_gpio.h"
#include "soc/lp_timer_reg.h"
#include "soc/rtc.h"
#include "ina219.h"
#include "types.h"

// NOTE: All global types are uint32_t, application must handle casting

// Cross processor state mangement
#define PRG_STATE_NONE 0
#define PRG_STATE_RUNNING 1
#define PRG_STATE_SLEEPING 2

#define SAMPLING_DELAY_WAIT 75 * 1000
#define MAX_BUS 2

// State variables
volatile state_t state = {0};

// ina219 configuration variables
volatile bus_cfg_t bus_cfg[MAX_BUS] = {0};
volatile general_cfg_t cfg = {0};

// Output variables
volatile bus_values_t bus[MAX_BUS] = {0};

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

static void accumulate(volatile bus_cfg_t *c, volatile bus_values_t *b, float previous_current, float previous_power,
                       uint32_t slow_clk_period) {
  float current = clamp(b->current, c->current_accum_threshold);
  float power = clamp(b->power, c->power_accum_threshold);
  uint64_t current_time = lp_core_rtc_ticks_to_us(lp_core_get_rtc_ticks(), slow_clk_period);
  float time_delta_hours = (current_time - b->last_sample_time) / 3600000000.0f;

  float charge = 0.f, energy = 0.f;
  if (b->last_sample_time != 0 && current_time > b->last_sample_time) {
    charge = ((current + previous_current) / 2.0f) * time_delta_hours;
    energy = ((power + previous_power) / 2.0f) * time_delta_hours;
    b->energy += energy;
    b->charge += charge;
  }

  if (c->reset) {
    // This ensures that the present sample is not lost
    // when reset is called.
    b->energy = energy;
    b->charge = charge;
  }

  b->last_sample_time = current_time;
}

static void process() {
  // The adc is set to 128 samples which takes around 68 ms
  // to finish according as per the datasheet. We wait a few
  // ms more.
  ulp_lp_core_delay_us(SAMPLING_DELAY_WAIT);

  float previous_current = 0.f, previous_power = 0.f;
  for (uint8_t i = 0; i < MAX_BUS; ++i) {
    volatile bus_values_t *b = &bus[i];
    volatile bus_cfg_t *c = &bus_cfg[i];

    if (c->address > 0 && b->error_code == ESP_OK) {
      previous_current = b->current;
      previous_power = b->power;

      // Sample sensor
      ina219_bus_voltage(c->address, &b->voltage);
      ina219_power(c->address, &b->power, b->current_lsb);
      ina219_shunt_voltage(c->address, &b->shunt_voltage);
      ina219_current(c->address, &b->current, b->current_lsb);

      // Compute min max
      min_max(b->current, &b->current_min, &b->current_max, c->reset);
      min_max(b->power, &b->power_min, &b->power_max, c->reset);
      min_max(b->voltage, &b->voltage_min, &b->voltage_max, c->reset);

      // Accumulate current and energy
      accumulate(c, b, previous_current, previous_power, state.slow_clk_period);

      // Clear reset if set
      c->reset = 0;

      // Power down
      ina219_power_down(c->address);
    }
  }
}

static void init() {
  esp_err_t ret = ESP_OK;
  uint32_t calibration_register = 0;
  uint32_t current_lsb = 0;

  for (uint8_t i = 0; i < MAX_BUS; ++i) {
    volatile bus_values_t *b = &bus[i];
    volatile bus_cfg_t *c = &bus_cfg[i];

    b->error_code = ESP_OK;
    if (c->address > 0) {
      ret = ina219_init(c->address, c->max_system_voltage, c->shunt_resistance, c->max_system_current,
                        c->calibration_register, &calibration_register, &current_lsb);
      if (ret == ESP_OK) {
        b->current_lsb = current_lsb;
        b->calibration_register = calibration_register;
      } else {
        b->error_code = ret;
      }
    }
  }
}

int main(void) {
  state.prg_state = PRG_STATE_RUNNING;
  uint64_t current = lp_core_get_rtc_ticks();

  if (cfg.led_interval > 0 && cfg.led_gpio > -1) {
    if (state.led_interval_counter >= cfg.led_interval) {
      ulp_lp_core_gpio_set_level(cfg.led_gpio, 1);
      state.led_interval_counter = 0;
    }
  }

  init();
  process();

  if (cfg.led_interval > 0 && cfg.led_gpio > -1) {
    state.led_interval_counter++;
    ulp_lp_core_gpio_set_level(cfg.led_gpio, 0);
  }

  state.run_duration = lp_core_rtc_ticks_to_us(lp_core_get_rtc_ticks() - current, state.slow_clk_period);
  state.prg_state = PRG_STATE_SLEEPING;

  return 0;
}
