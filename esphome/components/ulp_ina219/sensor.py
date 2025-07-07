import os

from esphome import automation, pins
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
    CONF_ABOVE,
    CONF_ADDRESS,
    CONF_BELOW,
    CONF_CURRENT,
    CONF_ID,
    CONF_INTERVAL,
    CONF_MAX_CURRENT,
    CONF_MAX_VOLTAGE,
    CONF_MODE,
    CONF_PIN,
    CONF_POWER,
    CONF_SHUNT_RESISTANCE,
    CONF_SHUNT_VOLTAGE,
    CONF_SLEEP_DURATION,
    CONF_THRESHOLD,
    CONF_VALUE,
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
    CONF_ACTIVITY_LED,
    CONF_BUS,
    CONF_BUS_A,
    CONF_BUS_B,
    CONF_CALIBRATION_REGISTER_OVERRIDE,
    CONF_CHARGE_DELTA,
    CONF_CHARGE_IN,
    CONF_CHARGE_NET,
    CONF_CHARGE_OUT,
    CONF_CURRENT_ACCUM_THRESHOLD,
    CONF_CURRENT_MAX,
    CONF_CURRENT_MIN,
    CONF_ENERGY_DELTA,
    CONF_ENERGY_IN,
    CONF_ENERGY_NET,
    CONF_ENERGY_OUT,
    CONF_LP_GPIO_PINS,
    CONF_LP_SCL,
    CONF_LP_SCL_PULLUP_ENABLED,
    CONF_LP_SDA,
    CONF_LP_SDA_PULLUP_ENABLED,
    CONF_POWER_ACCUM_THRESHOLD,
    CONF_POWER_MAX,
    CONF_POWER_MIN,
    CONF_TRIGGERS,
    CONF_VOLTAGE_MAX,
    CONF_VOLTAGE_MIN,
    UNIT_AMPS_HOURS,
)

DEPENDENCIES = ["esp32"]

ULP_FILES = ["main.c", "ina219.h", "ina219.c", "i2c.h", "i2c.c", "ulp_types.h"]

BUS_KEYS = [CONF_BUS_A, CONF_BUS_B]

ulp_ina219_sensor_ns = cg.esphome_ns.namespace("ulp_ina219")

UlpIna219SensorComponent = ulp_ina219_sensor_ns.class_("UlpIna219", cg.PollingComponent)

SetNetChargeAction = ulp_ina219_sensor_ns.class_(
    "SetNetChargeAction", automation.Action
)

SetNetEnergyAction = ulp_ina219_sensor_ns.class_(
    "SetNetEnergyAction", automation.Action
)

ResetValuesAction = ulp_ina219_sensor_ns.class_("ResetValuesAction", automation.Action)

ResetTriggersAction = ulp_ina219_sensor_ns.class_(
    "ResetTriggersAction", automation.Action
)

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
        cv.Optional(CONF_CALIBRATION_REGISTER_OVERRIDE, default=0): cv.All(
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
        cv.Optional(CONF_CHARGE_IN): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPS_HOURS,
            accuracy_decimals=4,
            state_class=STATE_CLASS_TOTAL,
        ),
        cv.Optional(CONF_ENERGY_IN): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT_HOURS,
            accuracy_decimals=4,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL,
        ),
        cv.Optional(CONF_CHARGE_OUT): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPS_HOURS,
            accuracy_decimals=4,
            state_class=STATE_CLASS_TOTAL,
        ),
        cv.Optional(CONF_ENERGY_OUT): sensor.sensor_schema(
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


TRIGGER_MODES = {
    CONF_VOLTAGE: "TRIG_MODE_VOLTAGE",
    CONF_CURRENT: "TRIG_MODE_CURRENT",
    CONF_CHARGE_NET: "TRIG_MODE_CHARGE_NET",
    CONF_CHARGE_IN: "TRIG_MODE_CHARGE_IN",
    CONF_CHARGE_OUT: "TRIG_MODE_CHARGE_OUT",
    CONF_ENERGY_NET: "TRIG_MODE_ENERGY_NET",
    CONF_ENERGY_IN: "TRIG_MODE_ENERGY_IN",
    CONF_ENERGY_OUT: "TRIG_MODE_ENERGY_OUT",
    CONF_CHARGE_DELTA: "TRIG_MODE_CHARGE_DELTA",
    CONF_ENERGY_DELTA: "TRIG_MODE_ENERGY_DELTA",
}

DELTA_MODES = [CONF_CHARGE_DELTA, CONF_ENERGY_DELTA]

RANGE_MODES = [key for key in TRIGGER_MODES.keys() if key not in DELTA_MODES]


def validate_trigger(value):
    mode = value["mode"]

    if mode in RANGE_MODES:
        if CONF_ABOVE not in value and CONF_BELOW not in value:
            raise cv.Invalid(f"Mode '{mode}' requires 'above' and/or 'below'")
        if CONF_THRESHOLD in value:
            raise cv.Invalid(f"Mode '{mode}' cannot have 'threshold'")

    elif mode in DELTA_MODES:
        if CONF_THRESHOLD not in value:
            raise cv.Invalid(f"Mode '{mode}' requires 'threshold'")
        if CONF_ABOVE in value or CONF_BELOW in value:
            raise cv.Invalid(f"Mode '{mode}' cannot have 'above' or 'below'")

    return value


TRIGGER_SCHEMA = cv.All(
    {
        cv.Required(CONF_MODE): cv.one_of(*TRIGGER_MODES.keys(), lower=True),
        cv.Optional(CONF_ABOVE): cv.float_,
        cv.Optional(CONF_BELOW): cv.float_,
        cv.Optional(CONF_THRESHOLD): cv.positive_float,
    },
    validate_trigger,
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
            cv.Optional(CONF_TRIGGERS, default=[]): cv.All(
                cv.ensure_list(TRIGGER_SCHEMA), cv.Length(max=3)
            ),
        }
    )
    .extend(CONFIG_SCHEMA)
    .extend(SENSOR_SCHEMA)
)

BUS_SCHEMA_B = (
    cv.Schema(
        {
            cv.Optional(CONF_ADDRESS, default=0x41): cv.hex_uint8_t,
            cv.Optional(CONF_TRIGGERS, default=[]): cv.All(
                cv.ensure_list(TRIGGER_SCHEMA), cv.Length(max=3)
            ),
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
            cv.Optional(CONF_ACTIVITY_LED): LED_SCHEMA,
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

SET_NET_CHARGE_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(UlpIna219SensorComponent),
        cv.Required(CONF_VALUE): cv.float_,
        cv.Required(CONF_BUS): cv.All(
            cv.uint8_t, cv.Range(min=0, max=len(BUS_KEYS) - 1)
        ),
    }
)

SET_NET_ENERGY_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(UlpIna219SensorComponent),
        cv.Required(CONF_VALUE): cv.float_,
        cv.Required(CONF_BUS): cv.All(
            cv.uint8_t, cv.Range(min=0, max=len(BUS_KEYS) - 1)
        ),
    }
)

RESET_VALUES_ACTION_SCHEMA = cv.maybe_simple_value(
    {
        cv.GenerateID(): cv.use_id(UlpIna219SensorComponent),
        cv.Required(CONF_BUS): cv.All(
            cv.uint8_t, cv.Range(min=0, max=len(BUS_KEYS) - 1)
        ),
    },
    key=CONF_BUS,
)

RESET_TRIGGERS_ACTION_SCHEMA = cv.maybe_simple_value(
    {
        cv.GenerateID(): cv.use_id(UlpIna219SensorComponent),
        cv.Required(CONF_BUS): cv.All(
            cv.uint8_t, cv.Range(min=0, max=len(BUS_KEYS) - 1)
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
    "ulp_ina219.reset_values",
    ResetTriggersAction,
    RESET_VALUES_ACTION_SCHEMA,
)
async def set_reset_triggers_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_bus_idx(config[CONF_BUS]))
    return var


CONFIG_TYPES = {
    CONF_ADDRESS: "set_address",
    CONF_SHUNT_RESISTANCE: "set_shunt_resistance",
    CONF_MAX_VOLTAGE: "set_max_system_voltage",
    CONF_MAX_CURRENT: "set_max_system_current",
    CONF_CURRENT_ACCUM_THRESHOLD: "set_current_accum_threshold",
    CONF_POWER_ACCUM_THRESHOLD: "set_power_accum_threshold",
    CONF_CALIBRATION_REGISTER_OVERRIDE: "set_calibration_register_override",
}

SENSOR_TYPES = {
    CONF_VOLTAGE: "set_voltage_sensor",
    CONF_CURRENT: "set_current_sensor",
    CONF_POWER: "set_power_sensor",
    CONF_ENERGY_NET: "set_energy_net_sensor",
    CONF_CHARGE_NET: "set_charge_net_sensor",
    CONF_ENERGY_IN: "set_energy_in_sensor",
    CONF_CHARGE_IN: "set_charge_in_sensor",
    CONF_ENERGY_OUT: "set_energy_out_sensor",
    CONF_CHARGE_OUT: "set_charge_out_sensor",
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
    esp32.add_idf_sdkconfig_option("CONFIG_ULP_COPROC_RESERVE_MEM", 13824)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_lp_sda_pullup_en(config[CONF_LP_SDA_PULLUP_ENABLED]))
    cg.add(var.set_lp_scl_pullup_en(config[CONF_LP_SCL_PULLUP_ENABLED]))

    sda = await cg.gpio_pin_expression(config[CONF_LP_SDA])
    cg.add(var.set_lp_sda_pin(sda))

    scl = await cg.gpio_pin_expression(config[CONF_LP_SCL])
    cg.add(var.set_lp_scl_pin(scl))

    if CONF_ACTIVITY_LED in config:
        if CONF_PIN in config[CONF_ACTIVITY_LED]:
            pin = await cg.gpio_pin_expression(config[CONF_ACTIVITY_LED][CONF_PIN])
            cg.add(var.set_led_pin(pin))
        if CONF_INTERVAL in config[CONF_ACTIVITY_LED]:
            cg.add(var.set_led_interval(config[CONF_ACTIVITY_LED][CONF_INTERVAL]))

    if CONF_SLEEP_DURATION in config:
        cg.add(var.set_sleep_duration(config[CONF_SLEEP_DURATION]))

    for bus_idx, bus_key in enumerate(BUS_KEYS):
        if bus_key in config:
            bus_config = config[bus_key]

            for key, fn in CONFIG_TYPES.items():
                if key in bus_config:
                    cg.add(getattr(var, fn)(bus_idx, bus_config[key]))

            for key, fn in SENSOR_TYPES.items():
                if key in bus_config:
                    s = await sensor.new_sensor(bus_config[key])
                    cg.add(getattr(var, fn)(bus_idx, s))

            if CONF_TRIGGERS in bus_config:
                trigger_configs = bus_config[CONF_TRIGGERS]
                cg.add(var.enable_ulp_wake_src())
                for tc in trigger_configs:
                    print(f"{tc}")
