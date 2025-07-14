import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_CURRENT,
    CONF_ID,
    CONF_POWER,
    CONF_SHUNT_VOLTAGE,
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

from .. import MAX_INA219_DEVICES, UlpIna219Component, ulp_ina219_ns
from ..const import (
    CONF_BUS,
    CONF_CHARGE_IN,
    CONF_CHARGE_NET,
    CONF_CHARGE_OUT,
    CONF_CURRENT_MAX,
    CONF_CURRENT_MIN,
    CONF_ENERGY_IN,
    CONF_ENERGY_NET,
    CONF_ENERGY_OUT,
    CONF_POWER_MAX,
    CONF_POWER_MIN,
    CONF_ULP_INA219_ID,
    CONF_VOLTAGE_MAX,
    CONF_VOLTAGE_MIN,
    UNIT_AMPS_HOURS,
)

DEPENDENCIES = ["ulp_ina219"]
CODEOWNERS = ["@bhaskarkishore"]

UlpIna219Sensor = ulp_ina219_ns.class_("UlpIna219Sensor", cg.PollingComponent)

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


def validate_index(value):
    if CONF_BUS not in value:
        raise cv.Invalid("must specify a bus index")
    if value[CONF_BUS] >= MAX_INA219_DEVICES:
        raise cv.Invalid(f"bus index must be between 0 and {MAX_INA219_DEVICES}")
    return value


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(UlpIna219Sensor),
            cv.GenerateID(CONF_ULP_INA219_ID): cv.use_id(UlpIna219Component),
            cv.Required(CONF_BUS): cv.positive_int,
        }
    )
    .extend(SENSOR_SCHEMA)
    .extend(cv.polling_component_schema("60s")),
)

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
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_ULP_INA219_ID])

    cg.add(var.set_bus_index(config[CONF_BUS]))

    for key, fn in SENSOR_TYPES.items():
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, fn)(sens))
