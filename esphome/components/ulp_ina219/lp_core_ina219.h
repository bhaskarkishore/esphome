
/* ULP variable definitions for the compiler.
 * This file is generated automatically by esp32ulp_mapgen.py utility.
 */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
extern uint32_t ulp___addsf3;
extern uint32_t ulp___ashldi3;
extern uint32_t ulp_bus_calibration_register;
extern uint32_t ulp_bus_current;
extern uint32_t ulp_bus_current_lsb;
extern uint32_t ulp_bus_current_mah;
extern uint32_t ulp_bus_current_max;
extern uint32_t ulp_bus_current_min;
extern uint32_t ulp_bus_error_code;
extern uint32_t ulp_bus_power;
extern uint32_t ulp_bus_power_max;
extern uint32_t ulp_bus_power_min;
extern uint32_t ulp_bus_reset;
extern uint32_t ulp_bus_shunt_voltage;
extern uint32_t ulp_bus_voltage;
extern uint32_t ulp_bus_voltage_max;
extern uint32_t ulp_bus_voltage_min;
extern uint32_t ulp_cfg_bus_address;
extern uint32_t ulp_cfg_bus_calibration_register;
extern uint32_t ulp_cfg_bus_current_clamp_threshold;
extern uint32_t ulp_cfg_bus_enabled;
extern uint32_t ulp_cfg_bus_max_current;
extern uint32_t ulp_cfg_bus_max_voltage;
extern uint32_t ulp_cfg_bus_shunt_resistance;
extern uint32_t ulp___clzsi2;
extern uint32_t ulp___clz_tab;
extern uint32_t ulp_dev;
extern uint32_t ulp___divdf3;
extern uint32_t ulp___divsf3;
extern uint32_t ulp___extendsfdf2;
extern uint32_t ulp___fixsfsi;
extern uint32_t ulp___fixunsdfsi;
extern uint32_t ulp___floatsisf;
extern uint32_t ulp___floatundisf;
extern uint32_t ulp___floatunsidf;
extern uint32_t ulp___floatunsisf;
extern uint32_t ulp___gesf2;
extern uint32_t ulp___gtsf2;
extern uint32_t ulp_i2c_read;
extern uint32_t ulp_i2c_read16;
extern uint32_t ulp_i2c_write;
extern uint32_t ulp_i2c_write16;
extern uint32_t ulp_ina219_bus_voltage;
extern uint32_t ulp_ina219_current;
extern uint32_t ulp_ina219_init;
extern uint32_t ulp_ina219_power;
extern uint32_t ulp_ina219_power_down;
extern uint32_t ulp_ina219_shunt_voltage;
extern uint32_t ulp__interrupt_handler;
extern uint32_t ulp___lesf2;
extern uint32_t ulp_LP_AON;
extern uint32_t ulp_LP_CLKRST;
extern uint32_t ulp_lp_core_i2c_master_read_from_device;
extern uint32_t ulp_lp_core_i2c_master_write_to_device;
extern uint32_t ulp_lp_core_startup;
extern uint32_t ulp_LP_I2C;
extern uint32_t ulp_LP_IO;
extern uint32_t ulp_LPPERI;
extern uint32_t ulp_LP_TIMER;
extern uint32_t ulp_LP_UART;
extern uint32_t ulp___lshrdi3;
extern uint32_t ulp___ltsf2;
extern uint32_t ulp_main;
extern uint32_t ulp___mtvec_base;
extern uint32_t ulp___muldf3;
extern uint32_t ulp___mulsf3;
extern uint32_t ulp__panic_handler;
extern uint32_t ulp_PCR;
extern uint32_t ulp_PMU;
extern uint32_t ulp_prg_state;
extern uint32_t ulp_reset_vector;
extern uint32_t ulp_run_duration;
extern uint32_t ulp_slow_clk_period;
extern uint32_t ulp___stack_top;
extern uint32_t ulp_UART0;
extern uint32_t ulp_ulp_lp_core_abort;
extern uint32_t ulp_ulp_lp_core_delay_cycles;
extern uint32_t ulp_ulp_lp_core_delay_us;
extern uint32_t ulp_ulp_lp_core_efuse_intr_handler;
extern uint32_t ulp_ulp_lp_core_halt;
extern uint32_t ulp_ulp_lp_core_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_adc_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_ana_peri_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_i2c_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_io_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_pmu_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_rtc_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_spi_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_sysreg_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_timer_get_cycle_count;
extern uint32_t ulp_ulp_lp_core_lp_timer_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_timer_set_wakeup_ticks;
extern uint32_t ulp_ulp_lp_core_lp_touch_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_uart_intr_handler;
extern uint32_t ulp_ulp_lp_core_lp_wdt_intr_handler;
extern uint32_t ulp_ulp_lp_core_mailbox_intr_handler;
extern uint32_t ulp_ulp_lp_core_memory_shared_cfg_get;
extern uint32_t ulp_ulp_lp_core_panic_handler;
extern uint32_t ulp_ulp_lp_core_stop_lp_core;
extern uint32_t ulp_ulp_lp_core_sw_intr_handler;
extern uint32_t ulp_ulp_lp_core_trng_intr_handler;
extern uint32_t ulp_ulp_lp_core_tsens_intr_handler;
extern uint32_t ulp_ulp_lp_core_update_wakeup_cause;
extern uint32_t ulp__vector_table;

#ifdef __cplusplus
}
#endif
