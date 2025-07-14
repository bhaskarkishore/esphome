import os

from esphome import automation, pins
import esphome.codegen as cg
from esphome.components import esp32
from esphome.components.esp32 import (
    VARIANT_ESP32C5,
    VARIANT_ESP32C6,
    get_esp32_variant,
    only_on_variant,
)
import esphome.config_validation as cv
from esphome.const import (
    CONF_ADDRESS,
    CONF_ID,
    CONF_INTERVAL,
    CONF_MAX_CURRENT,
    CONF_MAX_VOLTAGE,
    CONF_PIN,
    CONF_SCL,
    CONF_SDA,
    CONF_SHUNT_RESISTANCE,
    CONF_SLEEP_DURATION,
    CONF_VALUE,
)

from .const import (
    CONF_ACTIVITY_LED,
    CONF_BUS,
    CONF_CALIBRATION_REGISTER_OVERRIDE,
    CONF_CURRENT_ACCUM_THRESHOLD,
    CONF_LP_GPIO_PINS,
    CONF_LP_I2C,
    CONF_POWER_ACCUM_THRESHOLD,
)

DEPENDENCIES = ["esp32"]
CODEOWNERS = ["@bhaskarkishore"]

ULP_FILES = ["main.c", "ina219.h", "ina219.c", "i2c.h", "i2c.c", "common.h"]
MAX_INA219_DEVICES = 2

ulp_ina219_ns = cg.esphome_ns.namespace("ulp_ina219")

UlpIna219Component = ulp_ina219_ns.class_("UlpIna219", cg.Component)

SetNetChargeAction = ulp_ina219_ns.class_("SetNetChargeAction", automation.Action)

SetNetEnergyAction = ulp_ina219_ns.class_("SetNetEnergyAction", automation.Action)

ResetValuesAction = ulp_ina219_ns.class_("ResetValuesAction", automation.Action)

ResetTriggersAction = ulp_ina219_ns.class_("ResetTriggersAction", automation.Action)

SUPPORTED_VARIANTS = [VARIANT_ESP32C6, VARIANT_ESP32C5]

LP_PIN_CONFIG = {
    VARIANT_ESP32C6: {
        CONF_SDA: 6,
        CONF_SCL: 7,
        CONF_LP_GPIO_PINS: [0, 1, 2, 3, 4, 5],
    },
    VARIANT_ESP32C5: {
        CONF_SDA: 2,
        CONF_SCL: 3,
        CONF_LP_GPIO_PINS: [0, 1, 4, 5, 6],
    },
}

BUS_CONFIG_ITEMS = {
    CONF_ADDRESS: "set_address",
    CONF_SHUNT_RESISTANCE: "set_shunt_resistance",
    CONF_MAX_VOLTAGE: "set_max_system_voltage",
    CONF_MAX_CURRENT: "set_max_system_current",
    CONF_CURRENT_ACCUM_THRESHOLD: "set_current_accum_threshold",
    CONF_POWER_ACCUM_THRESHOLD: "set_power_accum_threshold",
    CONF_CALIBRATION_REGISTER_OVERRIDE: "set_calibration_register_override",
}


def validate_lp_gpio_pin(value):
    value = pins.gpio_output_pin_schema(value)
    variant = get_esp32_variant()
    lp_pins = LP_PIN_CONFIG.get(variant)
    if lp_pins is None:
        raise cv.Invalid("Unsupported variant {variant}")
    if value["number"] not in lp_pins[CONF_LP_GPIO_PINS]:
        raise cv.Invalid(
            f"lp led pin must be one of gpio pins: {lp_pins[CONF_LP_GPIO_PINS]} on {variant}"
        )
    return value


def validate_lp_sda_pin(value):
    value = pins.gpio_output_pin_schema(value)
    variant = get_esp32_variant()
    lp_pins = LP_PIN_CONFIG.get(variant)
    if lp_pins is None:
        raise cv.Invalid("Unsupported variant {variant}")
    if lp_pins[CONF_SDA] != value["number"]:
        raise cv.Invalid(f"lp sda pin must be gpio {lp_pins[CONF_SDA]} on {variant}")
    return value


def validate_lp_scl_pin(value):
    value = pins.gpio_output_pin_schema(value)
    variant = get_esp32_variant()
    lp_pins = LP_PIN_CONFIG.get(variant)
    if lp_pins is None:
        raise cv.Invalid("Unsupported variant {variant}")
    if lp_pins[CONF_SCL] != value["number"]:
        raise cv.Invalid(f"lp scl pin must be gpio {lp_pins[CONF_SCL]} on {variant}")
    return value


BUS_CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ADDRESS): cv.hex_uint8_t,
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
        cv.Optional(CONF_CALIBRATION_REGISTER_OVERRIDE, default=0): cv.All(
            cv.positive_int, cv.Range(min=0, max=32767)
        ),
    }
)

ACTIVITY_LED_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_PIN): validate_lp_gpio_pin,
        cv.Optional(CONF_INTERVAL, default=10): cv.All(
            cv.positive_int, cv.Range(min=0, max=255)
        ),
    }
)

LP_I2C_SCHEMA = cv.Schema(
    {
        cv.SplitDefault(
            CONF_SDA,
            esp32_c6=LP_PIN_CONFIG[VARIANT_ESP32C6][CONF_SDA],
            esp32_c5=LP_PIN_CONFIG[VARIANT_ESP32C5][CONF_SDA],
        ): validate_lp_sda_pin,
        cv.SplitDefault(
            CONF_SCL,
            esp32_c6=LP_PIN_CONFIG[VARIANT_ESP32C6][CONF_SCL],
            esp32_c5=LP_PIN_CONFIG[VARIANT_ESP32C5][CONF_SCL],
        ): validate_lp_scl_pin,
    }
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(UlpIna219Component),
            cv.Required(CONF_BUS): cv.All(
                cv.ensure_list(BUS_CONFIG_SCHEMA),
                cv.Length(min=1, max=MAX_INA219_DEVICES),
            ),
            cv.Optional(CONF_ACTIVITY_LED): ACTIVITY_LED_SCHEMA,
            cv.Optional("lp_i2c", default={}): LP_I2C_SCHEMA,
            cv.Optional(CONF_SLEEP_DURATION, default="177ms"): cv.templatable(
                cv.positive_time_period_milliseconds
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.only_with_esp_idf,
    only_on_variant(supported=SUPPORTED_VARIANTS),
)

SET_NET_CHARGE_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(UlpIna219Component),
        cv.Required(CONF_VALUE): cv.float_,
        cv.Required(CONF_BUS): cv.All(
            cv.uint8_t, cv.Range(min=0, max=MAX_INA219_DEVICES - 1)
        ),
    }
)

SET_NET_ENERGY_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(UlpIna219Component),
        cv.Required(CONF_VALUE): cv.float_,
        cv.Required(CONF_BUS): cv.All(
            cv.uint8_t, cv.Range(min=0, max=MAX_INA219_DEVICES - 1)
        ),
    }
)

RESET_VALUES_ACTION_SCHEMA = cv.maybe_simple_value(
    {
        cv.GenerateID(): cv.use_id(UlpIna219Component),
        cv.Required(CONF_BUS): cv.All(
            cv.uint8_t, cv.Range(min=0, max=MAX_INA219_DEVICES - 1)
        ),
    },
    key=CONF_BUS,
)

RESET_TRIGGERS_ACTION_SCHEMA = cv.maybe_simple_value(
    {
        cv.GenerateID(): cv.use_id(UlpIna219Component),
        cv.Required(CONF_BUS): cv.All(
            cv.uint8_t, cv.Range(min=0, max=MAX_INA219_DEVICES - 1)
        ),
    },
    key=CONF_BUS,
)


@automation.register_action(
    "ulp_ina219.set_net_charge", SetNetChargeAction, SET_NET_CHARGE_ACTION_SCHEMA
)
async def set_net_charge_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_value(config[CONF_VALUE]))
    cg.add(var.set_bus_idx(config[CONF_BUS]))
    return var


@automation.register_action(
    "ulp_ina219.set_net_energy", SetNetEnergyAction, SET_NET_ENERGY_ACTION_SCHEMA
)
async def set_net_energy_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_value(config[CONF_VALUE]))
    cg.add(var.set_bus_idx(config[CONF_BUS]))
    return var


@automation.register_action(
    "ulp_ina219.reset_values",
    ResetValuesAction,
    RESET_VALUES_ACTION_SCHEMA,
)
async def set_reset_values_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_bus_idx(config[CONF_BUS]))
    return var


@automation.register_action(
    "ulp_ina219.reset_triggers",
    ResetTriggersAction,
    RESET_VALUES_ACTION_SCHEMA,
)
async def set_reset_triggers_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_bus_idx(config[CONF_BUS]))
    return var


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
    esp32.add_idf_sdkconfig_option("CONFIG_ULP_COPROC_RESERVE_MEM", 14100)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    sda = await cg.gpio_pin_expression(config[CONF_LP_I2C][CONF_SDA])
    cg.add(var.set_lp_sda_pin(sda))

    scl = await cg.gpio_pin_expression(config[CONF_LP_I2C][CONF_SCL])
    cg.add(var.set_lp_scl_pin(scl))

    if CONF_ACTIVITY_LED in config:
        if CONF_PIN in config[CONF_ACTIVITY_LED]:
            pin = await cg.gpio_pin_expression(config[CONF_ACTIVITY_LED][CONF_PIN])
            cg.add(var.set_led_pin(pin))
        if CONF_INTERVAL in config[CONF_ACTIVITY_LED]:
            cg.add(var.set_led_interval(config[CONF_ACTIVITY_LED][CONF_INTERVAL]))

    if CONF_SLEEP_DURATION in config:
        cg.add(var.set_sleep_duration(config[CONF_SLEEP_DURATION]))

    for bus_idx, bus_config in enumerate(config[CONF_BUS]):
        for key, fn in BUS_CONFIG_ITEMS.items():
            if key in bus_config:
                cg.add(getattr(var, fn)(bus_idx, bus_config[key]))
