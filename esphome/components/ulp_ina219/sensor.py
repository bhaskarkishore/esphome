import os

import esphome.codegen as cg
from esphome.components import esp32, sensor
from esphome.components.esp32.const import VARIANT_ESP32C6, VARIANT_ESP32H2
import esphome.config_validation as cv
from esphome.const import (
    CONF_ADDRESS,
    CONF_CURRENT,
    CONF_ENERGY,
    CONF_ID,
    CONF_MAX_CURRENT,
    CONF_MAX_VOLTAGE,
    CONF_POWER,
    CONF_SHUNT_RESISTANCE,
    CONF_SHUNT_VOLTAGE,
    CONF_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLTAGE,
    KEY_CORE,
    KEY_TARGET_PLATFORM,
    KEY_VARIANT,
    PLATFORM_ESP32,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL,
    UNIT_AMPERE,
    UNIT_MILLISECOND,
    UNIT_VOLT,
    UNIT_WATT,
    UNIT_WATT_HOURS,
)
from esphome.core import CORE

from .const import (
    CONF_BUS_A,
    CONF_BUS_B,
    CONF_CHARGE,
    CONF_CURRENT_ACCUM_THRESHOLD,
    CONF_CURRENT_MAX,
    CONF_CURRENT_MIN,
    CONF_POWER_ACCUM_THRESHOLD,
    CONF_POWER_MAX,
    CONF_POWER_MIN,
    CONF_ULP_RUNTIME,
    CONF_VOLTAGE_MAX,
    CONF_VOLTAGE_MIN,
    UNIT_AMPS_HOURS,
)

DEPENDENCIES = ["esp32"]

ULP_FILES = ["main.c", "ina219.h", "ina219.c", "i2c.h", "i2c.c"]

ulp_ina219_sensor_ns = cg.esphome_ns.namespace("ulp_ina219")

UlpIna219SensorComponent = ulp_ina219_sensor_ns.class_("UlpIna219", cg.PollingComponent)

SUPPORTED_CHIPS = [VARIANT_ESP32C6, VARIANT_ESP32H2]


def validate_chip_variant(config):
    """Validate that the current chip is supported"""
    if CORE.data[KEY_CORE][KEY_TARGET_PLATFORM] != PLATFORM_ESP32:
        raise cv.Invalid("This component only supports ESP32 platform")

    chip_variant = CORE.data[PLATFORM_ESP32].get(KEY_VARIANT, "").upper()
    if chip_variant not in SUPPORTED_CHIPS:
        raise cv.Invalid(
            f"This component only supports {', '.join(SUPPORTED_CHIPS)} chips. Current chip: {chip_variant}"
        )

    return config


CONFIG_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_SHUNT_RESISTANCE, default=0.1): cv.All(
            cv.resistance, cv.Range(min=0.0, max=32.0)
        ),
        cv.Optional(CONF_MAX_VOLTAGE, default=32.0): cv.All(
            cv.voltage, cv.Range(min=0.0, max=32.0)
        ),
        cv.Optional(CONF_MAX_CURRENT, default=3.2): cv.All(
            cv.current, cv.Range(min=0.0)
        ),
        cv.Optional(CONF_CURRENT_ACCUM_THRESHOLD, default=0): cv.All(
            cv.current, cv.Range(min=0.0)
        ),
        cv.Optional(CONF_POWER_ACCUM_THRESHOLD, default=0): cv.All(
            cv.current, cv.Range(min=0.0)
        ),
    }
)

SENSOR_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_CURRENT): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_POWER): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SHUNT_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_CHARGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPS_HOURS,
            accuracy_decimals=4,
            state_class=STATE_CLASS_TOTAL,
        ),
        cv.Optional(CONF_ENERGY): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT_HOURS,
            accuracy_decimals=4,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL,
        ),
        cv.Required(CONF_CURRENT_MIN): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_VOLTAGE_MIN): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_POWER_MIN): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Required(CONF_CURRENT_MAX): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_VOLTAGE_MAX): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_POWER_MAX): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)

BUS_SCHEMA_A = (
    cv.Schema(
        {
            cv.Optional(CONF_ADDRESS, default=0x40): cv.hex_uint8_t,
        }
    )
    .extend(CONFIG_SCHEMA)
    .extend(SENSOR_SCHEMA)
)

BUS_SCHEMA_B = (
    cv.Schema(
        {
            cv.Optional(CONF_ADDRESS, default=0x41): cv.hex_uint8_t,
        }
    )
    .extend(CONFIG_SCHEMA)
    .extend(SENSOR_SCHEMA)
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(UlpIna219SensorComponent),
            cv.Optional("update_interval", default="60s"): cv.update_interval,
            cv.Required(CONF_BUS_A): BUS_SCHEMA_A,
            cv.Optional(CONF_BUS_B): BUS_SCHEMA_B,
            cv.Optional(CONF_ULP_RUNTIME): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLISECOND,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_DURATION,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    ).extend(cv.polling_component_schema("60s")),
    cv.has_at_least_one_key(CONF_BUS_A),
    validate_chip_variant,
)


async def to_code(config):
    """Generate the component code"""

    esp32.add_extra_build_file(
        "src/CMakeLists.txt",
        os.path.join(os.path.dirname(__file__), "CMakeLists.txt"),
    )

    for filename in ULP_FILES:
        f = f"ulp/{filename}"
        esp32.add_extra_build_file(
            f,
            os.path.join(os.path.dirname(__file__), f),
        )

    esp32.add_idf_sdkconfig_option("CONFIG_ULP_COPROC_ENABLED", True)
    esp32.add_idf_sdkconfig_option("CONFIG_ULP_COPROC_TYPE_LP_CORE", True)
    esp32.add_idf_sdkconfig_option("CONFIG_ULP_COPROC_RESERVE_MEM", 12288)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    bus_keys = [CONF_BUS_A, CONF_BUS_B]

    for bus_idx, bus_key in enumerate(bus_keys):
        if bus_key in config:
            bus_config = config[bus_key]

            cg.add(var.set_bus_enabled(bus_idx))

            if CONF_ADDRESS in bus_config:
                cg.add(var.set_address(bus_idx, bus_config[CONF_ADDRESS]))

            if CONF_SHUNT_RESISTANCE in bus_config:
                cg.add(
                    var.set_shunt_resistance(bus_idx, bus_config[CONF_SHUNT_RESISTANCE])
                )

            if CONF_MAX_VOLTAGE in bus_config:
                cg.add(
                    var.set_max_system_voltage(bus_idx, bus_config[CONF_MAX_VOLTAGE])
                )

            if CONF_MAX_CURRENT in bus_config:
                cg.add(
                    var.set_max_system_current(bus_idx, bus_config[CONF_MAX_CURRENT])
                )

            if CONF_CURRENT_ACCUM_THRESHOLD in bus_config:
                cg.add(
                    var.set_current_accum_threshold(
                        bus_idx, bus_config[CONF_CURRENT_ACCUM_THRESHOLD]
                    )
                )

            if CONF_POWER_ACCUM_THRESHOLD in bus_config:
                cg.add(
                    var.set_power_accum_threshold(
                        bus_idx, bus_config[CONF_POWER_ACCUM_THRESHOLD]
                    )
                )

            if CONF_VOLTAGE in bus_config:
                voltage_sensor = await sensor.new_sensor(bus_config[CONF_VOLTAGE])
                cg.add(var.set_voltage_sensor(bus_idx, voltage_sensor))

            if CONF_CURRENT in bus_config:
                current_sensor = await sensor.new_sensor(bus_config[CONF_CURRENT])
                cg.add(var.set_current_sensor(bus_idx, current_sensor))

            if CONF_POWER in bus_config:
                power_sensor = await sensor.new_sensor(bus_config[CONF_POWER])
                cg.add(var.set_power_sensor(bus_idx, power_sensor))

            if CONF_ENERGY in bus_config:
                energy_sensor = await sensor.new_sensor(bus_config[CONF_ENERGY])
                cg.add(var.set_energy_sensor(bus_idx, energy_sensor))

            if CONF_CHARGE in bus_config:
                charge_sensor = await sensor.new_sensor(bus_config[CONF_CHARGE])
                cg.add(var.set_charge_sensor(bus_idx, charge_sensor))

            if CONF_SHUNT_VOLTAGE in bus_config:
                shunt_voltage_sensor = await sensor.new_sensor(
                    bus_config[CONF_SHUNT_VOLTAGE]
                )
                cg.add(var.set_shunt_voltage_sensor(bus_idx, shunt_voltage_sensor))

            if CONF_VOLTAGE_MAX in bus_config:
                voltage_max_sensor = await sensor.new_sensor(
                    bus_config[CONF_VOLTAGE_MAX]
                )
                cg.add(var.set_voltage_max_sensor(bus_idx, voltage_max_sensor))

            if CONF_VOLTAGE_MIN in bus_config:
                voltage_min_sensor = await sensor.new_sensor(
                    bus_config[CONF_VOLTAGE_MIN]
                )
                cg.add(var.set_voltage_min_sensor(bus_idx, voltage_min_sensor))

            if CONF_CURRENT_MAX in bus_config:
                current_max_sensor = await sensor.new_sensor(
                    bus_config[CONF_CURRENT_MAX]
                )
                cg.add(var.set_current_max_sensor(bus_idx, current_max_sensor))

            if CONF_CURRENT_MIN in bus_config:
                current_min_sensor = await sensor.new_sensor(
                    bus_config[CONF_CURRENT_MIN]
                )
                cg.add(var.set_current_min_sensor(bus_idx, current_min_sensor))

            if CONF_POWER_MAX in bus_config:
                power_max_sensor = await sensor.new_sensor(bus_config[CONF_POWER_MAX])
                cg.add(var.set_power_max_sensor(bus_idx, power_max_sensor))

            if CONF_POWER_MIN in bus_config:
                power_min_sensor = await sensor.new_sensor(bus_config[CONF_POWER_MIN])
                cg.add(var.set_power_min_sensor(bus_idx, power_min_sensor))

    if CONF_ULP_RUNTIME in config:
        duration_sensor = await sensor.new_sensor(config[CONF_ULP_RUNTIME])
        cg.add(var.set_ulp_run_duration_sensor(duration_sensor))
