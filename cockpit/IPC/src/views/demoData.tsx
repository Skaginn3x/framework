import { RJSFSchema } from '@rjsf/utils';

export function demoRJSFSchema(): RJSFSchema {
  const data: RJSFSchema = {
    type: 'object',
    properties: {
      config: {
        $ref: '#/$defs/std::vector<Pin>',
      },
    },
    $defs: {
      Pin: {
        type: 'object',
        properties: {
          direction: {
            $ref: '#/$defs/tfc::observable<Pin direction>',
            description: 'Input or Output',
            default: 'as_is',
          },
        },
        required: [
          'direction',
        ],
        dependencies: {
          direction: {
            oneOf: [
              {
                properties: {
                  direction: {
                    enum: [
                      'input',
                    ],
                  },
                  bias: {
                    $ref: '#/$defs/tfc::observable<Pin voltage bias>',
                    default: 'as_is',
                  },
                  edge: {
                    $ref: '#/$defs/tfc::observable<Pin edge detection>',
                    default: 'none',
                  },
                },
              },
              {
                properties: {
                  direction: {
                    enum: [
                      'output',
                    ],
                  },
                  drive: {
                    $ref: '#/$defs/tfc::observable<Pin transistor driver setup>',
                  },
                  force: {
                    $ref: '#/$defs/tfc::observable<Pin output force state>',
                  },
                },
                required: [
                  'drive',
                  'force',
                ],
              },
              {
                properties: {
                  direction: {
                    enum: [
                      'as_is',
                    ],
                  },
                },
              },
            ],
          },
        },
      },
      'std::vector<Pin>': {
        type: 'array',
        items: {
          $ref: '#/$defs/Pin',
        },
      },
      'tfc::observable<Pin direction>': {
        type: 'string',
        oneOf: [
          {
            const: 'as_is',
          },
          {
            const: 'input',
          },
          {
            const: 'output',
          },
        ],
      },
      'tfc::observable<Pin edge detection>': {
        type: 'string',
        oneOf: [
          {
            const: 'none',
          },
          {
            const: 'rising',
          },
          {
            const: 'falling',
          },
          {
            const: 'both',
          },
        ],
      },
      'tfc::observable<Pin output force state>': {
        type: 'string',
        oneOf: [
          {
            const: 'as_is',
          },
          {
            const: 'on',
          },
          {
            const: 'off',
          },
          {
            const: 'save_on',
          },
          {
            const: 'save_off',
          },
        ],
      },
      'tfc::observable<Pin transistor driver setup>': {
        type: 'string',
        oneOf: [
          {
            const: 'push_pull',
          },
          {
            const: 'open_drain',
          },
          {
            const: 'open_source',
          },
        ],
      },
      'tfc::observable<Pin voltage bias>': {
        type: 'string',
        oneOf: [
          {
            const: 'as_is',
          },
          {
            const: 'unknown',
          },
          {
            const: 'disabled',
          },
          {
            const: 'pull_up',
          },
          {
            const: 'pull_down',
          },
        ],
      },
    },
  };
  return data;
}

export function demoRJSFData2(): RJSFSchema {
  const data: RJSFSchema = {
    nominal_motor_power: {
      value: {
        value: 15,
        unit: 'hW',
        dimension: 'power',
        ratio: {
          numerator: 100,
          denominator: 1,
        },
      },
      index: [
        8258,
        14,
      ],
      name: 'NPR',
      desc: 'Nominal motor power',
    },
    nominal_motor_voltage: {
      value: {
        value: 400,
        unit: 'V',
        dimension: 'voltage',
        ratio: {
          numerator: 1,
          denominator: 1,
        },
      },
      index: [
        8258,
        2,
      ],
      name: 'UNS',
      desc: 'Nominal motor voltage',
    },
    nominal_motor_current: {
      value: {
        value: 20,
        unit: 'dA',
        dimension: 'electric_current',
        ratio: {
          numerator: 1,
          denominator: 10,
        },
      },
      index: [
        8258,
        4,
      ],
      name: 'NCR',
      desc: 'Nominal motor current',
    },
    nominal_motor_frequency: {
      value: {
        value: 500,
        unit: 'dHz',
        dimension: 'frequency',
        ratio: {
          numerator: 1,
          denominator: 10,
        },
      },
      index: [
        8258,
        3,
      ],
      name: 'FRS',
      desc: 'Nominal motor frequency',
    },
    nominal_motor_speed: {
      value: 1500,
      index: [
        8258,
        5,
      ],
      name: 'NSP',
      desc: 'Nominal motor speed',
    },
    max_frequency: {
      value: {
        value: 800,
        unit: 'dHz',
        dimension: 'frequency',
        ratio: {
          numerator: 1,
          denominator: 10,
        },
      },
      index: [
        8193,
        4,
      ],
      name: 'TFR',
      desc: 'Max frequency',
    },
    motor_thermal_current: {
      value: {
        value: 20,
        unit: 'dA',
        dimension: 'electric_current',
        ratio: {
          numerator: 1,
          denominator: 10,
        },
      },
      index: [
        8258,
        23,
      ],
      name: 'ITH',
      desc: 'motor thermal current',
    },
    high_speed: {
      value: {
        value: 800,
        unit: 'dHz',
        dimension: 'frequency',
        ratio: {
          numerator: 1,
          denominator: 10,
        },
      },
      index: [
        8193,
        5,
      ],
      name: 'HSP',
      desc: 'High speed',
    },
    low_speed: {
      value: {
        value: 200,
        unit: 'dHz',
        dimension: 'frequency',
        ratio: {
          numerator: 1,
          denominator: 10,
        },
      },
      index: [
        8193,
        6,
      ],
      name: 'LSP',
      desc: 'Low speed',
    },
    cos_phi: {
      value: 80,
      index: [
        8258,
        7,
      ],
      name: 'COS',
      desc: 'Motor 1 cosinus phi',
    },
    acceleration: {
      value: {
        value: 1,
        unit: 'ds',
        dimension: 'time',
        ratio: {
          numerator: 1,
          denominator: 10,
        },
      },
      index: [
        8252,
        2,
      ],
      name: 'ACC',
      desc: 'Acceleration ramp time',
    },
    deceleration: {
      value: {
        value: 1,
        unit: 'ds',
        dimension: 'time',
        ratio: {
          numerator: 1,
          denominator: 10,
        },
      },
      index: [
        8252,
        3,
      ],
      name: 'DEC',
      desc: 'Deceleration time ramp',
    },
  };
  return data;
}

export function demoRJSFSchema2(): RJSFSchema {
  const data: RJSFSchema = {
    type: [
      'object',
    ],
    properties: {
      config: {
        $ref: '#/$defs/tfc::observable<atv320>',
      },
    },
    additionalProperties: false,
    $defs: {
      'glz::unknown': {
        type: [
          'number',
          'string',
          'boolean',
          'object',
          'array',
          'null',
        ],
      },
      'std::chrono::duration<uint16_t,std::deci>': {
        type: [
          'integer',
        ],
      },
      'std::string_view': {
        type: [
          'string',
        ],
      },
      'tfc::ec::setting::ACC': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/std::chrono::duration<uint16_t,std::deci>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::COS': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/uint16_t',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::DEC': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/std::chrono::duration<uint16_t,std::deci>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::FRS': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/units::quantity<frequency,dHz,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::HSP': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/units::quantity<frequency,dHz,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::ITH': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/units::quantity<electric_current,dA,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::LSP': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/units::quantity<frequency,dHz,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::NCR': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/units::quantity<electric_current,dA,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::NPR': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/units::quantity<power,hW,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::NSP': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/uint16_t',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::TFR': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/units::quantity<frequency,dHz,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::ec::setting::UNS': {
        type: [
          'object',
        ],
        properties: {
          desc: {
            $ref: '#/$defs/std::string_view',
          },
          index: {
            $ref: '#/$defs/glz::unknown',
          },
          name: {
            $ref: '#/$defs/std::string_view',
          },
          value: {
            $ref: '#/$defs/units::quantity<voltage,V,uint16_t>',
          },
        },
        additionalProperties: false,
      },
      'tfc::observable<atv320>': {
        type: [
          'object',
        ],
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
        type: [
          'integer',
        ],
      },
      'units::quantity<electric_current,dA,uint16_t>': {
        type: [
          'integer',
        ],
      },
      'units::quantity<frequency,dHz,uint16_t>': {
        type: [
          'integer',
        ],
      },
      'units::quantity<power,hW,uint16_t>': {
        type: [
          'integer',
        ],
      },
      'units::quantity<voltage,V,uint16_t>': {
        type: [
          'integer',
        ],
      },
    },
  };
  return data;
}

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
