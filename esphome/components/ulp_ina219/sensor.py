import os

import esphome.codegen as cg
from esphome.components import esp32, sensor
from esphome.components.esp32.const import VARIANT_ESP32C6, VARIANT_ESP32H2
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
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
    CONF_BUS_A_CHARGE,
    CONF_BUS_A_CURRENT,
    CONF_BUS_A_ENERGY,
    CONF_BUS_A_POWER,
    CONF_BUS_A_SHUNT_VOLTAGE,
    CONF_BUS_A_VOLTAGE,
    CONF_ULP_RUNTIME,
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


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(UlpIna219SensorComponent),
            cv.Required(CONF_BUS_A_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BUS_A_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BUS_A_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BUS_A_SHUNT_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BUS_A_CHARGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPS_HOURS,
                accuracy_decimals=4,
                state_class=STATE_CLASS_TOTAL,
            ),
            cv.Optional(CONF_BUS_A_ENERGY): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT_HOURS,
                accuracy_decimals=4,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_TOTAL,
            ),
            cv.Optional(CONF_ULP_RUNTIME): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLISECOND,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_DURATION,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("update_interval", default="60s"): cv.update_interval,
        }
    ).extend(cv.polling_component_schema("60s")),
    cv.has_at_least_one_key(CONF_BUS_A_VOLTAGE),
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

    if CONF_BUS_A_VOLTAGE in config:
        voltage_sensor = await sensor.new_sensor(config[CONF_BUS_A_VOLTAGE])
        cg.add(var.set_voltage_sensor(0, voltage_sensor))

    if CONF_BUS_A_CURRENT in config:
        current_sensor = await sensor.new_sensor(config[CONF_BUS_A_CURRENT])
        cg.add(var.set_current_sensor(0, current_sensor))

    if CONF_BUS_A_POWER in config:
        power_sensor = await sensor.new_sensor(config[CONF_BUS_A_POWER])
        cg.add(var.set_power_sensor(0, power_sensor))

    if CONF_BUS_A_ENERGY in config:
        energy_sensor = await sensor.new_sensor(config[CONF_BUS_A_ENERGY])
        cg.add(var.set_energy_sensor(0, energy_sensor))

    if CONF_BUS_A_CHARGE in config:
        charge_sensor = await sensor.new_sensor(config[CONF_BUS_A_CHARGE])
        cg.add(var.set_charge_sensor(0, charge_sensor))

    if CONF_BUS_A_SHUNT_VOLTAGE in config:
        shunt_voltage_sensor = await sensor.new_sensor(config[CONF_BUS_A_SHUNT_VOLTAGE])
        cg.add(var.set_shunt_voltage_sensor(0, shunt_voltage_sensor))

    if CONF_ULP_RUNTIME in config:
        duration_sensor = await sensor.new_sensor(config[CONF_ULP_RUNTIME])
        cg.add(var.set_duration_sensor(0, duration_sensor))
