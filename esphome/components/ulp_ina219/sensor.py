import os

import esphome.codegen as cg
from esphome.components import esp32, sensor
from esphome.components.esp32.const import VARIANT_ESP32C6, VARIANT_ESP32H2
import esphome.config_validation as cv
from esphome.const import (
    CONF_CURRENT,
    CONF_ID,
    CONF_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_VOLTAGE,
    KEY_CORE,
    KEY_TARGET_PLATFORM,
    KEY_VARIANT,
    PLATFORM_ESP32,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_VOLT,
)
from esphome.core import CORE

DEPENDENCIES = ["esp32"]

ULP_FILES = ["main.c", "ina219.h", "ina219.c", "i2c.c", "i2c.h"]

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
            cv.Required(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("update_interval", default="60s"): cv.update_interval,
        }
    ).extend(cv.polling_component_schema("60s")),
    cv.has_at_least_one_key(CONF_VOLTAGE, CONF_CURRENT),
    validate_chip_variant,
)


async def to_code(config):
    """Generate the component code"""

    esp32.add_extra_build_file(
        "src/esphome/components/ulp_ina219/CMakeLists.txt",
        os.path.join(os.path.dirname(__file__), "CMakeLists.txt"),
    )

    for filename in ULP_FILES:
        f = f"lp_core/{filename}"
        esp32.add_extra_build_file(
            f,
            os.path.join(os.path.dirname(__file__), f),
        )

    esp32.add_idf_sdkconfig_option("CONFIG_ULP_COPROC_ENABLED", True)
    esp32.add_idf_sdkconfig_option("CONFIG_ULP_COPROC_TYPE_LP_CORE", True)
    esp32.add_idf_sdkconfig_option("CONFIG_ULP_COPROC_RESERVE_MEM", 12288)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_VOLTAGE in config:
        voltage_sensor = await sensor.new_sensor(config[CONF_VOLTAGE])
        cg.add(var.set_voltage_sensor(voltage_sensor))

    if CONF_CURRENT in config:
        current_sensor = await sensor.new_sensor(config[CONF_CURRENT])
        cg.add(var.set_current_sensor(current_sensor))
