import os

from esphome import pins
import esphome.codegen as cg
from esphome.components import esp32, sensor
from esphome.components.esp32 import (
    VARIANT_ESP32C5,
    VARIANT_ESP32C6,
    get_esp32_variant,
    only_on_variant,
)
import esphome.config_validation as cv
from esphome.const import (
    CONF_ADDRESS,
    CONF_CURRENT,
    CONF_ID,
    CONF_INTERVAL,
    CONF_LED,
    CONF_MAX_CURRENT,
    CONF_MAX_VOLTAGE,
    CONF_PIN,
    CONF_POWER,
    CONF_SHUNT_RESISTANCE,
    CONF_SHUNT_VOLTAGE,
    CONF_SLEEP_DURATION,
    CONF_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL,
    UNIT_AMPERE,
    UNIT_VOLT,
    UNIT_WATT,
    UNIT_WATT_HOURS,
)

from .const import (
    CONF_BUS_A,
    CONF_BUS_B,
    CONF_CALIBRATION_REGISTER,
    CONF_CHARGE_NET,
    CONF_CURRENT_ACCUM_THRESHOLD,
    CONF_CURRENT_MAX,
    CONF_CURRENT_MIN,
    CONF_ENERGY_NET,
    CONF_LP_GPIO_PINS,
    CONF_LP_SCL,
    CONF_LP_SCL_PULLUP_ENABLED,
    CONF_LP_SDA,
    CONF_LP_SDA_PULLUP_ENABLED,
    CONF_POWER_ACCUM_THRESHOLD,
    CONF_POWER_MAX,
    CONF_POWER_MIN,
    CONF_VOLTAGE_MAX,
    CONF_VOLTAGE_MIN,
    UNIT_AMPS_HOURS,
)

DEPENDENCIES = ["esp32"]
CODEOWNERS = ["@bhaskarkishore"]

ULP_FILES = ["main.c", "ina219.h", "ina219.c", "i2c.h", "i2c.c", "types.h"]

BUS_KEYS = [CONF_BUS_A, CONF_BUS_B]

ulp_ina219_sensor_ns = cg.esphome_ns.namespace("ulp_ina219")

UlpIna219SensorComponent = ulp_ina219_sensor_ns.class_("UlpIna219", cg.PollingComponent)

SUPPORTED_VARIANTS = [VARIANT_ESP32C6, VARIANT_ESP32C5]

LP_PIN_CONFIG = {
    VARIANT_ESP32C6: {
        CONF_LP_SDA: 6,
        CONF_LP_SCL: 7,
        CONF_LP_GPIO_PINS: [0, 1, 2, 3, 4, 5],
    },
    VARIANT_ESP32C5: {
        CONF_LP_SDA: 2,
        CONF_LP_SCL: 3,
        CONF_LP_GPIO_PINS: [0, 1, 4, 5, 6],
    },
}

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
        cv.Optional(CONF_CALIBRATION_REGISTER, default=0): cv.All(
            cv.positive_int, cv.Range(min=0, max=32767)
        ),
    }
)

SENSOR_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_CURRENT): sensor.sensor_schema(
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
        cv.Optional(CONF_CHARGE_NET): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPS_HOURS,
            accuracy_decimals=4,
            state_class=STATE_CLASS_TOTAL,
        ),
        cv.Optional(CONF_ENERGY_NET): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT_HOURS,
            accuracy_decimals=4,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL,
        ),
        cv.Optional(CONF_CURRENT_MIN): sensor.sensor_schema(
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
        cv.Optional(CONF_CURRENT_MAX): sensor.sensor_schema(
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


LED_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_PIN): validate_lp_gpio_pin,
        cv.Optional(CONF_INTERVAL, default=10): cv.All(
            cv.positive_int, cv.Range(min=0, max=255)
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


def validate_lp_sda_pin(value):
    value = pins.gpio_output_pin_schema(value)
    variant = get_esp32_variant()
    lp_pins = LP_PIN_CONFIG.get(variant)
    if lp_pins is None:
        raise cv.Invalid("Unsupported variant {variant}")
    if lp_pins[CONF_LP_SDA] != value["number"]:
        raise cv.Invalid(f"lp sda pin must be gpio {lp_pins[CONF_LP_SDA]} on {variant}")
    return value


def validate_lp_scl_pin(value):
    value = pins.gpio_output_pin_schema(value)
    variant = get_esp32_variant()
    lp_pins = LP_PIN_CONFIG.get(variant)
    if lp_pins is None:
        raise cv.Invalid("Unsupported variant {variant}")
    if lp_pins[CONF_LP_SCL] != value["number"]:
        raise cv.Invalid(f"lp scl pin must be gpio {lp_pins[CONF_LP_SCL]} on {variant}")
    return value


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(UlpIna219SensorComponent),
            cv.Optional("update_interval", default="60s"): cv.update_interval,
            cv.Optional(CONF_BUS_A): BUS_SCHEMA_A,
            cv.Optional(CONF_BUS_B): BUS_SCHEMA_B,
            cv.Optional(CONF_LED): LED_SCHEMA,
            cv.Optional(CONF_SLEEP_DURATION, default="177ms"): cv.templatable(
                cv.positive_time_period_milliseconds
            ),
            cv.SplitDefault(
                CONF_LP_SDA,
                esp32_c6=LP_PIN_CONFIG[VARIANT_ESP32C6][CONF_LP_SDA],
                esp32_c5=LP_PIN_CONFIG[VARIANT_ESP32C5][CONF_LP_SDA],
            ): validate_lp_sda_pin,
            cv.SplitDefault(
                CONF_LP_SCL,
                esp32_c6=LP_PIN_CONFIG[VARIANT_ESP32C6][CONF_LP_SCL],
                esp32_c5=LP_PIN_CONFIG[VARIANT_ESP32C5][CONF_LP_SCL],
            ): validate_lp_scl_pin,
            cv.Optional(CONF_LP_SDA_PULLUP_ENABLED, default=True): cv.boolean,
            cv.Optional(CONF_LP_SCL_PULLUP_ENABLED, default=True): cv.boolean,
        }
    ).extend(cv.polling_component_schema("60s")),
    cv.has_at_least_one_key(CONF_BUS_A, CONF_BUS_B),
    cv.only_with_esp_idf,
    only_on_variant(supported=SUPPORTED_VARIANTS),
)

CONFIG_TYPES = {
    CONF_ADDRESS: "set_address",
    CONF_SHUNT_RESISTANCE: "set_shunt_resistance",
    CONF_MAX_VOLTAGE: "set_max_system_voltage",
    CONF_MAX_CURRENT: "set_max_system_current",
    CONF_CURRENT_ACCUM_THRESHOLD: "set_current_accum_threshold",
    CONF_POWER_ACCUM_THRESHOLD: "set_power_accum_threshold",
    CONF_CALIBRATION_REGISTER: "set_calibration_register",
}

SENSOR_TYPES = {
    CONF_VOLTAGE: "set_voltage_sensor",
    CONF_CURRENT: "set_current_sensor",
    CONF_POWER: "set_power_sensor",
    CONF_ENERGY_NET: "set_energy_net_sensor",
    CONF_CHARGE_NET: "set_charge_net_sensor",
    CONF_SHUNT_VOLTAGE: "set_shunt_voltage_sensor",
    CONF_VOLTAGE_MAX: "set_voltage_max_sensor",
    CONF_VOLTAGE_MIN: "set_voltage_min_sensor",
    CONF_CURRENT_MAX: "set_current_max_sensor",
    CONF_CURRENT_MIN: "set_current_min_sensor",
    CONF_POWER_MAX: "set_power_max_sensor",
    CONF_POWER_MIN: "set_power_min_sensor",
}


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

    cg.add(var.set_lp_sda_pullup_en(config[CONF_LP_SDA_PULLUP_ENABLED]))
    cg.add(var.set_lp_scl_pullup_en(config[CONF_LP_SCL_PULLUP_ENABLED]))

    sda = await cg.gpio_pin_expression(config[CONF_LP_SDA])
    cg.add(var.set_lp_sda_pin(sda))

    scl = await cg.gpio_pin_expression(config[CONF_LP_SCL])
    cg.add(var.set_lp_scl_pin(scl))

    if CONF_LED in config:
        if CONF_PIN in config[CONF_LED]:
            pin = await cg.gpio_pin_expression(config[CONF_LED][CONF_PIN])
            cg.add(var.set_led_pin(pin))
        if CONF_INTERVAL in config[CONF_LED]:
            cg.add(var.set_led_interval(config[CONF_LED][CONF_INTERVAL]))

    if CONF_SLEEP_DURATION in config:
        cg.add(var.set_sleep_duration(config[CONF_SLEEP_DURATION]))

    for bus_idx, bus_key in enumerate(BUS_KEYS):
        if bus_key in config:
            bus_config = config[bus_key]
            cg.add(var.set_bus_enabled(bus_idx))

            for key, fn in CONFIG_TYPES.items():
                if key in bus_config:
                    cg.add(getattr(var, fn)(bus_idx, bus_config[key]))

            for key, fn in SENSOR_TYPES.items():
                if key in bus_config:
                    s = await sensor.new_sensor(bus_config[key])
                    cg.add(getattr(var, fn)(bus_idx, s))
