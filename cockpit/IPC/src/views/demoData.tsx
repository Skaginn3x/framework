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
                  type: ['string'],
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
                  type: ['string'],
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

export function ATVDemoData(): any {
  return {
    config: {
      nominal_motor_power: 0,
      nominal_motor_voltage: 400,
      nominal_motor_current: 20,
      nominal_motor_frequency: 500,
      nominal_motor_speed: 1500,
      max_frequency: 800,
      motor_thermal_current: 20,
      high_speed: 800,
      low_speed: 200,
      cos_phi: 80,
      acceleration: 1,
      deceleration: 1,
    },
  };
}

export function ATVDemoSchema(): any {
  return {
    type: ['object'],
    properties: { config: { $ref: '#/$defs/tfc::observable<atv320>' } },
    additionalProperties: false,
    $defs: {
      'tfc::ec::setting::ACC': {
        type: ['integer'],
        title: 'Acceleration ramp time',
        minimum: 0,
        maximum: 65535,
        'x-tfc': {
          unit: 'ds', dimension: 'time', ratio: { numerator: 1, denominator: 10 }, required: true,
        },
      },
      'tfc::ec::setting::COS': {
        type: ['integer'], title: 'Motor 1 cosinus phi', minimum: 0, maximum: 65535,
      },
      'tfc::ec::setting::DEC': {
        type: ['integer'],
        title: 'Deceleration time ramp',
        minimum: 0,
        maximum: 65535,
        'x-tfc': {
          unit: 'ds', dimension: 'time', ratio: { numerator: 1, denominator: 10 }, required: true,
        },
      },
      'tfc::ec::setting::FRS': {
        type: ['integer'],
        title: 'Nominal motor frequency',
        minimum: 0,
        maximum: 65535,
        'x-tfc': {
          unit: 'dHz', dimension: 'frequency', ratio: { numerator: 1, denominator: 10 }, required: true,
        },
      },
      'tfc::ec::setting::HSP': {
        type: ['integer'],
        title: 'High speed',
        minimum: 0,
        maximum: 65535,
        'x-tfc': {
          unit: 'dHz', dimension: 'frequency', ratio: { numerator: 1, denominator: 10 }, required: true,
        },
      },
      'tfc::ec::setting::ITH': {
        type: ['integer'],
        title: 'motor thermal current',
        minimum: 0,
        maximum: 65535,
        'x-tfc': {
          unit: 'dA', dimension: 'electric_current', ratio: { numerator: 1, denominator: 10 }, required: true,
        },
      },
      'tfc::ec::setting::LSP': {
        type: ['integer'],
        title: 'Low speed',
        minimum: 0,
        maximum: 65535,
        'x-tfc': {
          unit: 'dHz', dimension: 'frequency', ratio: { numerator: 1, denominator: 10 }, required: true,
        },
      },
      'tfc::ec::setting::NCR': {
        type: ['integer'],
        title: 'Nominal motor current',
        minimum: 0,
        maximum: 65535,
        'x-tfc': {
          unit: 'dA', dimension: 'electric_current', ratio: { numerator: 1, denominator: 10 }, required: true,
        },
      },
      'tfc::ec::setting::NPR': {
        type: ['integer'],
        title: 'Nominal motor power',
        minimum: 0,
        maximum: 65535,
        'x-tfc': {
          unit: 'hW', dimension: 'power', ratio: { numerator: 100, denominator: 1 }, required: true,
        },
      },
      'tfc::ec::setting::NSP': {
        type: ['integer'], title: 'Nominal motor speed', minimum: 0, maximum: 65535,
      },
      'tfc::ec::setting::TFR': {
        type: ['integer'],
        title: 'Max frequency',
        minimum: 0,
        maximum: 65535,
        'x-tfc': {
          unit: 'dHz', dimension: 'frequency', ratio: { numerator: 1, denominator: 10 }, required: true,
        },
      },
      'tfc::ec::setting::UNS': {
        type: ['integer'],
        title: 'Nominal motor voltage',
        minimum: 0,
        maximum: 65535,
        'x-tfc': {
          unit: 'V', dimension: 'voltage', ratio: { numerator: 1, denominator: 1 }, required: true,
        },
      },
      'tfc::observable<atv320>': {
        type: ['object'],
        properties: {
          acceleration: { $ref: '#/$defs/tfc::ec::setting::ACC' },
          cos_phi: { $ref: '#/$defs/tfc::ec::setting::COS' },
          deceleration: { $ref: '#/$defs/tfc::ec::setting::DEC' },
          high_speed: { $ref: '#/$defs/tfc::ec::setting::HSP' },
          low_speed: { $ref: '#/$defs/tfc::ec::setting::LSP' },
          max_frequency: { $ref: '#/$defs/tfc::ec::setting::TFR' },
          motor_thermal_current: { $ref: '#/$defs/tfc::ec::setting::ITH' },
          nominal_motor_current: { $ref: '#/$defs/tfc::ec::setting::NCR' },
          nominal_motor_frequency: { $ref: '#/$defs/tfc::ec::setting::FRS' },
          nominal_motor_power: { $ref: '#/$defs/tfc::ec::setting::NPR' },
          nominal_motor_speed: { $ref: '#/$defs/tfc::ec::setting::NSP' },
          nominal_motor_voltage: { $ref: '#/$defs/tfc::ec::setting::UNS' },
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

export function ArrayTestSchema(): any {
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

export function VariantSchema(): any {
  return {
    type: ['object'],
    properties: {
      a_int: {
        $ref: '#/$defs/int32_t',
        description: 'A int description',
      },
      variant: {
        $ref: '#/$defs/std::variant<std::monostate,option_1,option_2>',
        description: 'variant description',
      },
    },
    additionalProperties: false,
    $defs: {
      int32_t: { type: ['integer'] },
      'std::string': { type: ['string'] },
      'std::variant<std::monostate,option_1,option_2>': {
        type: ['number', 'string', 'boolean', 'object', 'array', 'null'],
        oneOf: [
          { type: ['null'], title: 'std::monostate' },
          {
            type: ['object'],
            title: 'option 1',
            properties: {
              amper: {
                $ref: '#/$defs/units::quantity<electric_current,dA,uint16_t>',
                description: 'amper description',
              },
            },
            additionalProperties: false,
          },
          {
            type: ['object'],
            title: 'option 2',
            properties: {
              a: {
                $ref: '#/$defs/std::string',
                description: 'A description',
              },
              sec: {
                $ref: '#/$defs/tfc::observable<std::chrono::duration<glz::unknown,std::nano>>',
                description: 'sec description',
              },
            },
            additionalProperties: false,
          },
        ],
      },
      'tfc::observable<std::chrono::duration<glz::unknown,std::nano>>': {
        type: ['integer'],
        'x-tfc': {
          unit: 'ns', dimension: 'time', ratio: { numerator: 1, denominator: 1000000000 }, required: true,
        },
      },
      'units::quantity<electric_current,dA,uint16_t>': {
        type: ['integer'],
        'x-tfc': {
          unit: 'dA', dimension: 'electric_current', ratio: { numerator: 1, denominator: 10 }, required: true,
        },
      },
    },
  };
}
export function VariantData(): any {
  return { a_int: 0, variant: { sec: 22 } };
}
