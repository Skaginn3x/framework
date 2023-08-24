export function demoUiSchema(): any {
  return {
    type: [
      'object',
    ],
    properties: {
      config: {
        $ref: '#/$defs/with_variant',
      },
    },
    additionalProperties: false,
    $defs: {
      int32_t: {
        type: [
          'integer',
        ],
      },
      'std::string': {
        type: [
          'string',
        ],
      },
      'std::variant<std::monostate,option_1,option_2>': {
        type: [
          'string',
        ],
        enum: [
          'None',
          'Option 1',
          'Option 2',
        ],
      },
      'tfc::observable<std::chrono::duration<int64_t,std::nano>>': {
        type: [
          'integer',
        ],
      },
      'units::quantity<electric_current,dA,uint16_t>': {
        type: [
          'integer',
        ],
      },
      with_variant: {
        type: [
          'object',
        ],
        properties: {
          a_int: {
            $ref: '#/$defs/int32_t',
            description: 'A int description',
          },
          variant: {
            $ref: '#/$defs/std::variant<std::monostate,option_1,option_2>',
            description: 'variant description',
            title: 'Options',
            default: 'None',
          },
        },
        allOf: [
          {
            if: {
              properties: {
                variant: {
                  type: 'string',
                  const: 'Option 1',
                },
              },
            },
            then: {
              properties: {
                amper: {
                  $ref: '#/$defs/units::quantity<electric_current,dA,uint16_t>',
                  description: 'amper description',
                },
              },
            },
          },
          {
            if: {
              properties: {
                variant: {
                  type: 'string',
                  const: 'Option 2',
                },
              },
            },
            then: {
              properties: {
                a: {
                  $ref: '#/$defs/std::string',
                  description: 'A description',
                },
                sec: {
                  $ref: '#/$defs/tfc::observable<std::chrono::duration<int64_t,std::nano>>',
                  description: 'sec description',
                },
              },
            },
          },
        ],
      },
    },
  };
}

export function demoUiSchema2(): any {
  return {
    type: ['object'],
    properties: { config: { $ref: '#/$defs/state_machine' } },
    additionalProperties: false,
    $defs: {
      state_machine: {
        type: ['object'],
        properties: {
          startup_time: {
            $ref: '#/$defs/std::optional<std::chrono::duration<int64_t,std::milli>>',
            description: '[ms] Delay to run initial sequences to get the equipment ready.',
          },
          stopping_time: {
            $ref: '#/$defs/std::optional<std::chrono::duration<int64_t,std::milli>>',
            description: '[ms] Delay to run ending sequences to get the equipment ready for being stopped.',
          },
        },
        additionalProperties: false,
      },
      'std::optional<std::chrono::duration<int64_t,std::milli>>': {
        type: ['integer', 'null'],
      },
    },
  };
}

export function demoUiData3(): any {
  return {
    nominal_motor_power: {
      value: {
        value: 15, unit: 'hW', dimension: 'power', ratio: { numerator: 100, denominator: 1 },
      },
      index: [8258, 14],
      name: 'NPR',
      desc: 'Nominal motor power',
    },
    nominal_motor_voltage: {
      value: {
        value: 400, unit: 'V', dimension: 'voltage', ratio: { numerator: 1, denominator: 1 },
      },
      index: [8258, 2],
      name: 'UNS',
      desc: 'Nominal motor voltage',
    },
    nominal_motor_current: {
      value: {
        value: 20, unit: 'dA', dimension: 'electric_current', ratio: { numerator: 1, denominator: 10 },
      },
      index: [8258, 4],
      name: 'NCR',
      desc: 'Nominal motor current',
    },
    nominal_motor_frequency: {
      value: {
        value: 500, unit: 'dHz', dimension: 'frequency', ratio: { numerator: 1, denominator: 10 },
      },
      index: [8258, 3],
      name: 'FRS',
      desc: 'Nominal motor frequency',
    },
    nominal_motor_speed: {
      value: 1500, index: [8258, 5], name: 'NSP', desc: 'Nominal motor speed',
    },
    max_frequency: {
      value: {
        value: 800, unit: 'dHz', dimension: 'frequency', ratio: { numerator: 1, denominator: 10 },
      },
      index: [8193, 4],
      name: 'TFR',
      desc: 'Max frequency',
    },
    motor_thermal_current: {
      value: {
        value: 20, unit: 'dA', dimension: 'electric_current', ratio: { numerator: 1, denominator: 10 },
      },
      index: [8258, 23],
      name: 'ITH',
      desc: 'motor thermal current',
    },
    high_speed: {
      value: {
        value: 800, unit: 'dHz', dimension: 'frequency', ratio: { numerator: 1, denominator: 10 },
      },
      index: [8193, 5],
      name: 'HSP',
      desc: 'High speed',
    },
    low_speed: {
      value: {
        value: 200, unit: 'dHz', dimension: 'frequency', ratio: { numerator: 1, denominator: 10 },
      },
      index: [8193, 6],
      name: 'LSP',
      desc: 'Low speed',
    },
    cos_phi: {
      value: 80, index: [8258, 7], name: 'COS', desc: 'Motor 1 cosinus phi',
    },
    acceleration: {
      value: {
        value: 1, unit: 'ds', dimension: 'time', ratio: { numerator: 1, denominator: 10 },
      },
      index: [8252, 2],
      name: 'ACC',
      desc: 'Acceleration ramp time',
    },
    deceleration: {
      value: {
        value: 1, unit: 'ds', dimension: 'time', ratio: { numerator: 1, denominator: 10 },
      },
      index: [8252, 3],
      name: 'DEC',
      desc: 'Deceleration time ramp',
    },
  };
}

export function demoUiSchema3(): any {
  return {
    type: ['object'],
    properties: {
      config: { $ref: '#/$defs/tfc::observable<glz::atv320>' },
    },
    additionalProperties: false,
    $defs: {
      'glz::unknown': {
        type: ['number', 'string', 'boolean', 'object', 'array', 'null'],
      },
      int64_t: {
        type: ['integer'],
      },
      'std::chrono::duration<uint16_t,std::deci>': {
        type: ['number'],
      },
      'std::string_view': {
        type: ['string'],
      },
      'tfc::ec::setting::ACC': {
        type: ['object'],
        properties: {
          value: {
            title: 'Acceleration ramp time',
            $ref: '#/$defs/std::chrono::duration<uint16_t,std::deci>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::COS': {
        type: ['object'],
        properties: {
          value: {
            title: 'Motor 1 cosinus phi',
            $ref: '#/$defs/uint16_t',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::DEC': {
        type: ['object'],
        properties: {
          value: {
            title: 'Deceleration duration',
            $ref: '#/$defs/std::chrono::duration<uint16_t,std::deci>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::FRS': {
        type: ['object'],
        properties: {
          value: {
            title: 'Nominal motor frequency',
            $ref: '#/$defs/units::quantity<frequency,dHz,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::HSP': {
        type: ['object'],
        properties: {
          value: {
            title: 'High speed frequency',
            $ref: '#/$defs/units::quantity<frequency,dHz,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::ITH': {
        type: ['object'],
        properties: {
          value: {
            title: 'Motor thermal current',
            $ref: '#/$defs/units::quantity<electric_current,dA,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::LSP': {
        type: ['object'],
        properties: {
          value: {
            title: 'Low speed frequency',
            $ref: '#/$defs/units::quantity<frequency,dHz,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::NCR': {
        type: ['object'],
        properties: {
          value: {
            title: 'Nominal motor current',
            $ref: '#/$defs/units::quantity<electric_current,dA,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::NPR': {
        type: ['object'],
        properties: {
          value: {
            title: 'Nominal motor power',
            $ref: '#/$defs/units::quantity<power,hW,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::NSP': {
        type: ['object'],
        properties: {
          value: {
            title: 'Nominal motor speed',
            $ref: '#/$defs/uint16_t',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::TFR': {
        type: ['object'],
        properties: {
          value: {
            title: 'Motor thermal frequency',
            $ref: '#/$defs/units::quantity<frequency,dHz,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::UNS': {
        type: ['object'],
        properties: {
          value: {
            title: 'Motor nominal voltage',
            $ref: '#/$defs/units::quantity<voltage,V,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::observable<glz::atv320>': {
        type: ['object'],
        properties: {
          acceleration: {
            $ref: '#/$defs/tfc::ec::setting::ACC',
          },
          cos_phi: {
            $ref: '#/$defs/tfc::ec::setting::COS',
          },
          deceleration: {
            $ref: '#/$defs/tfc::ec::setting::DEC',
          },
          high_speed: {
            $ref: '#/$defs/tfc::ec::setting::HSP',
          },
          low_speed: {
            $ref: '#/$defs/tfc::ec::setting::LSP',
          },
          max_frequency: {
            $ref: '#/$defs/tfc::ec::setting::TFR',
          },
          motor_thermal_current: {
            $ref: '#/$defs/tfc::ec::setting::ITH',
          },
          nominal_motor_current: {
            $ref: '#/$defs/tfc::ec::setting::NCR',
          },
          nominal_motor_frequency: {
            $ref: '#/$defs/tfc::ec::setting::FRS',
          },
          nominal_motor_power: {
            $ref: '#/$defs/tfc::ec::setting::NPR',
          },
          nominal_motor_speed: {
            $ref: '#/$defs/tfc::ec::setting::NSP',
          },
          nominal_motor_voltage: {
            $ref: '#/$defs/tfc::ec::setting::UNS',
          },
        },
        additionalProperties: false,
      },
      uint16_t: {
        type: ['integer'],
      },
      'units::quantity<electric_current,dA,uint16_t>': {
        type: ['object'],
        properties: {
          value: {
            $ref: '#/$defs/uint16_t',
          },
        },
        additionalProperties: false,
      },
      'units::quantity<frequency,dHz,uint16_t>': {
        type: ['object'],
        properties: {
          value: {
            $ref: '#/$defs/uint16_t',
          },
        },
        additionalProperties: false,
      },
      'units::quantity<power,hW,uint16_t>': {
        type: ['object'],
        properties: {
          value: {
            $ref: '#/$defs/uint16_t',
          },
        },
        additionalProperties: false,
      },
      'units::quantity<voltage,V,uint16_t>': {
        type: ['object'],
        properties: {
          value: {
            $ref: '#/$defs/uint16_t',
          },
        },
        additionalProperties: false,
      },
      'units::ratio': {
        type: ['object'],
        properties: {
          denominator: {
            $ref: '#/$defs/int64_t',
          },
          numerator: {

            $ref: '#/$defs/int64_t',
          },
        },
        additionalProperties: false,
      },
    },
  };
}

export function newDemoSchema1(): any {
  return {
    type: [
      'object',
    ],
    properties: {
      config: {
        $ref: '#/$defs/state_machine',
      },
    },
    additionalProperties: false,
    $defs: {
      state_machine: {
        type: [
          'object',
        ],
        properties: {
          startup_time2: {
            $ref: '#/$defs/std::optional<std::chrono::duration<int64,std::milli>>',
            description: '[ms] Delay to run',
            'x-tfc': {
              unit: 'ms',
              required: true,
              dimension: 'time',
              ratio: {
                numerator: 1,
                denominator: 1000,
              },
            },
          },
          stopping_time2: {
            $ref: '#/$defs/std::optional<std::chrono::duration<int64,std::milli>>',
            description: '[ms] Delay.',
            'x-tfc': {
              unit: 'ms',
              required: true,
              dimension: 'time',
              ratio: {
                numerator: 1,
                denominator: 1000,
              },
            },
          },
        },
        additionalProperties: false,
      },
      'std::optional<std::chrono::duration<int64,std::milli>>': {
        type: [
          'integer',
          'null',
        ],
      },
    },
  };
}

export function newDemoData1(): any {
  return {
    config: {
      startup_time2: 1200,
      stopping_time2: 800,
    },
  };
}

export function newDemoSchema2(): any {
  return {
    type: ['object'],
    properties: {
      config: {
        type: [
          'array',
        ],
        items: {
          $ref: '#/$defs/object_in_array',
        },
      },
    },
    $defs: {
      int32_t: {
        type: [
          'integer',
        ],
      },
      object_in_array: {
        type: [
          'object',
        ],
        view: {
          notSortable: true,
          notDeletable: true,
        },
        properties: {
          a_int: {
            $ref: '#/$defs/int32_t',
            description: 'A int description',
          },
          amper: {
            $ref: '#/$defs/units::quantity<electric_current,dA,uint16_t>',
            description: 'amper description',
          },
        },
        additionalProperties: false,
      },
      'units::quantity<electric_current,dA,uint16_t>': {
        type: [
          'integer',
        ],
        'x-tfc': {
          unit: 'dA',
          dimension: 'electric_current',
          ratio: {
            numerator: 1,
            denominator: 10,
          },
          required: true,
        },
      },
    },
  };
}
