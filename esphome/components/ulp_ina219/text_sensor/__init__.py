import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ABOVE,
    CONF_BELOW,
    CONF_CURRENT,
    CONF_DEBOUNCE,
    CONF_MODE,
    CONF_THRESHOLD,
    CONF_VOLTAGE,
)

from .. import UlpIna219Component, ulp_ina219_ns
from ..const import (
    CONF_ALARM_INDEX,
    CONF_BUS,
    CONF_CHARGE_DELTA,
    CONF_CHARGE_IN,
    CONF_CHARGE_NET,
    CONF_CHARGE_OUT,
    CONF_ENERGY_DELTA,
    CONF_ENERGY_IN,
    CONF_ENERGY_NET,
    CONF_ENERGY_OUT,
    CONF_ULP_INA219_ID,
)

DEPENDENCIES = ["ulp_ina219"]
CODEOWNERS = ["@bhaskarkishore"]


UlpIna219TextSensor = ulp_ina219_ns.class_(
    "UlpIna219TextSensor",
    text_sensor.TextSensor,
    cg.PollingComponent,
)


ALARM_MODES = {
    CONF_VOLTAGE: "ALARM_MODE_VOLTAGE",
    CONF_CURRENT: "ALARM_MODE_CURRENT",
    CONF_CHARGE_NET: "ALARM_MODE_CHARGE_NET",
    CONF_CHARGE_IN: "ALARM_MODE_CHARGE_IN",
    CONF_CHARGE_OUT: "ALARM_MODE_CHARGE_OUT",
    CONF_ENERGY_NET: "ALARM_MODE_ENERGY_NET",
    CONF_ENERGY_IN: "ALARM_MODE_ENERGY_IN",
    CONF_ENERGY_OUT: "ALARM_MODE_ENERGY_OUT",
    CONF_CHARGE_DELTA: "ALARM_MODE_CHARGE_DELTA",
    CONF_ENERGY_DELTA: "ALARM_MODE_ENERGY_DELTA",
}

DELTA_MODES = [CONF_CHARGE_DELTA, CONF_ENERGY_DELTA]

RANGE_MODES = [key for key in ALARM_MODES if key not in DELTA_MODES]


def validate_alarm(value):
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


CONFIG_SCHEMA = cv.All(
    text_sensor.text_sensor_schema(icon="mdi:alarm-light")
    .extend(
        {
            cv.GenerateID(): cv.declare_id(UlpIna219TextSensor),
            cv.GenerateID(CONF_ULP_INA219_ID): cv.use_id(UlpIna219Component),
            cv.Required(CONF_BUS): cv.All(cv.uint8_t, cv.Range(min=0, max=1)),
            cv.Required(CONF_ALARM_INDEX): cv.All(cv.uint8_t, cv.Range(min=0, max=3)),
            cv.Required(CONF_MODE): cv.one_of(*ALARM_MODES.keys(), lower=True),
            cv.Optional(CONF_ABOVE): cv.float_,
            cv.Optional(CONF_BELOW): cv.float_,
            cv.Optional(CONF_THRESHOLD): cv.positive_float,
            cv.Optional(
                CONF_DEBOUNCE, default="60s"
            ): cv.positive_time_period_milliseconds,
        }
    )
    .extend(cv.polling_component_schema("1s")),
    validate_alarm,
)


async def to_code(config):
    print(f"CONFIG: {config}")
    var = await text_sensor.new_text_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_ULP_INA219_ID])

    mode = ALARM_MODES[config[CONF_MODE]]
    above = config.get(CONF_ABOVE, cg.RawExpression("INFINITY"))
    below = config.get(CONF_BELOW, cg.RawExpression("-INFINITY"))
    threshold = config.get(CONF_THRESHOLD, cg.RawExpression("INFINITY"))
    cg.add(
        var.set_alarm(
            config[CONF_BUS],
            config[CONF_ALARM_INDEX],
            getattr(ulp_ina219_ns, mode),
            config[CONF_DEBOUNCE],
            above,
            below,
            threshold,
        )
    )
