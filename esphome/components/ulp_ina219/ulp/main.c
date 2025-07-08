#include "ulp_lp_core_gpio.h"
#include "ulp_lp_core_utils.h"
#include "ulp_lp_core_lp_timer_shared.h"
#include "ina219.h"
#include "ulp_types.h"
#include <math.h>
#include "esp_err.h"

ulp_ina219_context_t ctx = {0};
uint64_t start_ticks = 0;

static float clamp(float value, float threshold) { return fabs(value) <= threshold ? 0.f : value; }

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

static void accumulate(volatile const bus_config_t *c, volatile bus_values_t *v, float previous_current,
                       float previous_power) {
  float current = clamp(v->current, c->current_accum_threshold);
  float power = clamp(v->power, c->power_accum_threshold);
  uint64_t current_time = lp_core_rtc_ticks_to_us(lp_core_get_rtc_ticks(), ctx.slow_clk_period);
  float time_delta_hours = (current_time - v->last_sample_time) / 3600000000.0f;

  float charge = 0.f, charge_in = 0.f, charge_out = 0.f, energy = 0.f, energy_in = 0.f, energy_out = 0.f;
  if (v->last_sample_time != 0 && current_time > v->last_sample_time) {
    charge = ((current + previous_current) / 2.0f) * time_delta_hours;
    energy = ((power + previous_power) / 2.0f) * time_delta_hours;
    v->energy_net += energy;
    v->charge_net += charge;
    if (v->current < 0) {
      v->charge_out += fabs(charge);
      v->energy_out += fabs(energy);
    } else {
      v->charge_in += fabs(charge);
      v->energy_in += fabs(energy);
    }
  }

  if (c->reset) {
    // The following ensures that the present sample is not lost
    // when reset is called by the main cpu. This may help improve
    // accuracy in cases where the ulp cpu sleeps for long
    // durations.
    v->energy_net = energy;
    v->charge_net = charge;

    if (v->current < 0) {
      v->charge_in = 0.f;
      v->energy_in = 0.f;
      v->charge_out = fabs(charge);
      v->energy_out = fabs(energy);
    } else {
      v->charge_in = fabs(charge);
      v->energy_in = fabs(energy);
      v->charge_out = 0.f;
      v->energy_out = 0.f;
    }
  }

  v->last_sample_time = current_time;
}

static void check_triggers(uint8_t bus_idx) {
  volatile wake_trigger_t *t = ctx.buses[bus_idx].triggers;
  volatile bus_values_t *v = &ctx.buses[bus_idx].values;

  for (uint8_t i = 0; i < MAX_TRIGGERS; ++i) {
    bool triggered = false;

    if (t[i].status == TRIG_SET) {
      volatile wake_trigger_conditions_t *cond = &t[i].condition;

      switch (t[i].mode) {
        case TRIG_MODE_VOLTAGE:
          triggered = (v->voltage > cond->range.above || v->voltage < cond->range.below);
          break;
        case TRIG_MODE_CURRENT:
          triggered = (v->current > cond->range.above || v->current < cond->range.below);
          break;
        case TRIG_MODE_CHARGE_NET:
          triggered = (v->charge_net > cond->range.above || v->charge_net < cond->range.below);
          break;
        case TRIG_MODE_CHARGE_IN:
          triggered = (v->charge_in > cond->range.above || v->charge_in < cond->range.below);
          break;
        case TRIG_MODE_CHARGE_OUT:
          triggered = (v->charge_out > cond->range.above || v->charge_out < cond->range.below);
          break;
        case TRIG_MODE_ENERGY_NET:
          triggered = (v->energy_net > cond->range.above || v->energy_net < cond->range.below);
          break;
        case TRIG_MODE_ENERGY_IN:
          triggered = (v->energy_in > cond->range.above || v->energy_in < cond->range.below);
          break;
        case TRIG_MODE_ENERGY_OUT:
          triggered = (v->energy_out > cond->range.above || v->energy_out < cond->range.below);
          break;
        case TRIG_MODE_CHARGE_DELTA:
          if (cond->delta.baseline == INFINITY) {
            cond->delta.baseline = v->charge_net;
          } else {
            triggered = (fabs(v->charge_net - cond->delta.baseline) > cond->delta.threshold);
          }
          break;
        case TRIG_MODE_ENERGY_DELTA:
          if (cond->delta.baseline == INFINITY) {
            cond->delta.baseline = v->energy_net;
          } else {
            triggered = (fabs(v->energy_net - cond->delta.baseline) > cond->delta.threshold);
          }
          break;
        case TRIG_MODE_NONE:
          break;
        default:
          break;
      }

      if (triggered && !ctx.main_cpu_awake) {
        uint64_t current_time_us = lp_core_rtc_ticks_to_us(lp_core_get_rtc_ticks(), ctx.slow_clk_period);
        if ((current_time_us - t[i].last_fired_us) > t[i].debounce_us) {
          t[i].status = TRIG_FIRED;
          t[i].last_fired_us = current_time_us;
          ulp_lp_core_wakeup_main_processor();
          return;
        }
      }
    }
  }
}

static void update() {
  float previous_current = 0.f, previous_power = 0.f;
  for (uint8_t i = 0; i < MAX_BUS; ++i) {
    volatile bus_values_t *v = &ctx.buses[i].values;
    volatile bus_config_t *c = &ctx.buses[i].config;

    if (c->address > 0 && v->error_code == ESP_OK) {
      previous_current = v->current;
      previous_power = v->power;

      // Sample sensor
      ina219_bus_voltage(c->address, &v->voltage);
      ina219_power(c->address, &v->power, v->current_lsb);
      ina219_shunt_voltage(c->address, &v->shunt_voltage);
      ina219_current(c->address, &v->current, v->current_lsb);

      // Compute min max
      min_max(v->current, &v->current_min, &v->current_max, c->reset);
      min_max(v->power, &v->power_min, &v->power_max, c->reset);
      min_max(v->voltage, &v->voltage_min, &v->voltage_max, c->reset);

      // Accumulate current and energy
      accumulate(c, v, previous_current, previous_power);

      // Clear reset if set
      c->reset = false;

      // Power down
      ina219_power_down(c->address);

      // Check triggers
      if (ctx.triggers_enabled) {
        check_triggers(i);
      }
    }
  }
}

static void init() {
  esp_err_t ret = ESP_OK;
  uint32_t calibration_register = 0;
  uint32_t current_lsb = 0;

  for (uint8_t i = 0; i < MAX_BUS; ++i) {
    volatile bus_values_t *v = &ctx.buses[i].values;
    volatile bus_config_t *c = &ctx.buses[i].config;

    v->error_code = ESP_OK;
    if (c->address > 0) {
      ret = ina219_init(c->address, c->max_system_voltage, c->shunt_resistance, c->max_system_current,
                        c->calibration_register_override, &calibration_register, &current_lsb);
      if (ret == ESP_OK) {
        v->current_lsb = current_lsb;
        v->calibration_register = calibration_register;
      } else {
        v->error_code = ret;
      }
    }
  }
}

int main(void) {
  bool led_active = ctx.led.interval > 0 && ctx.led.pin > -1;

  // Skip init if the previous run cycle initiated a wait sleep.
  if (ctx.prg_state != PRG_STATE_WAIT_SLEEP) {
    start_ticks = lp_core_get_rtc_ticks();
    ctx.prg_state = PRG_STATE_RUN;

    if (led_active && ctx.led.counter >= ctx.led.interval) {
      ulp_lp_core_gpio_set_level(ctx.led.pin, 1);
      ctx.led.counter = 0;
    }

    // Initialize the devices
    init();

    // The adc is set to 128 samples per value which takes around 68 ms
    // to finish according as per the datasheet, we wait a little extra.
    // To save power, we put the ulp processor to sleep while we wait.
    ctx.prg_state = PRG_STATE_WAIT_SLEEP;
    uint64_t ticks = ulp_lp_core_lp_timer_calculate_sleep_ticks(SAMPLING_DELAY_WAIT_US);
    ulp_lp_core_lp_timer_set_wakeup_ticks(ticks);
    ulp_lp_core_halt();  // Execution stops here
  }

  ctx.prg_state = PRG_STATE_RUN;
  // Read devices and update values
  update();

  if (led_active) {
    ctx.led.counter++;
    ulp_lp_core_gpio_set_level(ctx.led.pin, 0);
  }

  ctx.run_duration =
      lp_core_rtc_ticks_to_us(lp_core_get_rtc_ticks() - start_ticks, ctx.slow_clk_period) - SAMPLING_DELAY_WAIT_US;

  ctx.prg_state = PRG_STATE_SLEEP;
  return 0;
}
