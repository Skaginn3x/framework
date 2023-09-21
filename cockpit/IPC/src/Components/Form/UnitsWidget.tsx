/* eslint-disable prefer-const */
/* eslint-disable no-continue */
/* eslint-disable react/require-default-props */
import React, {
  CSSProperties, FocusEventHandler, KeyboardEventHandler, MouseEventHandler,
} from 'react';
import TextField from '@mui/material/TextField';
import { InputProps } from '@mui/material/Input';
import {
  FormControl, InputLabel, MenuItem, Select, Tooltip,
} from '@mui/material';
import { useUID } from 'react-uid';
import { TransTitle } from '@ui-schema/ui-schema/Translate/TransTitle';
import { mapSchema } from '@ui-schema/ui-schema/Utils/schemaToNative';
import { schemaTypeIs } from '@ui-schema/ui-schema/Utils/schemaTypeIs';
import {
  WidgetProps, WithValue, useUIStore,
} from '@ui-schema/ui-schema';
import { MuiWidgetBinding } from '@ui-schema/ds-material/widgetsBinding';

import * as math from 'mathjs';
import { Units } from './units';
import { getNestedValue } from './WidgetFunctions';
import './UnitsWidget.css';

export interface UnitWidgetBaseProps {
  style?: CSSProperties
  onClick: MouseEventHandler<HTMLDivElement> | undefined
  onFocus: FocusEventHandler<HTMLInputElement | HTMLTextAreaElement> | undefined
  onBlur: FocusEventHandler<HTMLInputElement | HTMLTextAreaElement> | undefined
  onKeyUp: KeyboardEventHandler<HTMLDivElement> | undefined
  onKeyDown: KeyboardEventHandler<HTMLDivElement> | undefined
  steps?: number | 'any'
  inputProps?: InputProps['inputProps']
  InputProps?: Partial<InputProps>
  inputRef?: any
}

export function UnitWidget<P extends WidgetProps<MuiWidgetBinding> = WidgetProps<MuiWidgetBinding>>({
  storeKeys, schema, onChange, valid,
  style, onClick, onFocus, onBlur, onKeyUp, onKeyDown,
  // eslint-disable-next-line @typescript-eslint/no-shadow
  inputProps = {}, InputProps = {}, inputRef: customInputRef,
  steps = 'any',
}: P & UnitWidgetBaseProps & WithValue): React.ReactElement {
  const uid = useUID();
  const { store } = useUIStore();

  let initialDimension: string | undefined;
  let includesUnits = true;
  // eslint-disable-next-line no-param-reassign, @typescript-eslint/no-unused-vars
  valid = false;

  const storeValues = store ? store.toJS().values : {};

  const storeValue = getNestedValue(storeValues, storeKeys.toJS());
  initialDimension = schema.toJS()['x-tfc'] ? schema.toJS()['x-tfc'].dimension : undefined;
  const initialUnit = schema.toJS()['x-tfc'] ? schema.toJS()['x-tfc'].unit : 'NoUnit';
  const required = schema.toJS()['x-tfc'] ? schema.toJS()['x-tfc'].required : false;
  if (!initialDimension) {
    includesUnits = false;
  }
  let inputRef = React.useRef();
  if (customInputRef) {
    inputRef = customInputRef;
  }

  // eslint-disable-next-line no-param-reassign
  inputProps = mapSchema(inputProps, schema);
  const AllUnits = Units as { [key: string]: { all: string[], default: string } };

  // Maybe we could default to SI units using .toSI().toJSON() (properties: unit, value)
  // Could make it easier to configure.
  // Hz becomes s^-1, m/s becomes m*s^-1, etc. (https://mathjs.org/docs/datatypes/units.html)
  // We might want to override this to get Hz instead of s^-1, etc.

  const [unit, setUnit] = React.useState(initialUnit);
  const [value, setValue] = React.useState<string>(storeValue !== undefined ? storeValue : '');

  const handleUnitChange = (event: any) => {
    const newUnit = event.target.value;
    if (value !== null && value !== undefined && newUnit && initialUnit) {
      const floatValue = parseFloat(value);
      const valueInBaseUnit = math.unit(floatValue, unit).toNumber(initialUnit); // Convert current value to base unit (e.g. m)
      const newValueInNewUnit = math.round(math.unit(valueInBaseUnit, initialUnit).toNumber(newUnit), 3); // Convert value to new unit
      setValue(newValueInNewUnit.toString());
      setUnit(newUnit);

      onChange({
        storeKeys,
        scopes: ['value'],
        type: 'set',
        schema,
        required,
        data: {
          value:
            valueInBaseUnit,
        },
      });
    }
  };

  const hideTitle = schema.getIn(['view', 'hideTitle']);

  const schemaType = schema.get('type') as string | undefined;
  const newInputProps = React.useMemo(() => {
    if (schemaTypeIs(schemaType, 'number') && typeof inputProps.step === 'undefined') {
      return {
        ...inputProps,
        step: steps,
      };
    }
    return inputProps;
  }, [inputProps, steps, schemaType]);

  function getStyle() {
    return {
      ...style,
      width: includesUnits ? 'calc(80% - 0.5rem)' : '100%',
    };
  }

  // eslint-disable-next-line @typescript-eslint/no-shadow
  const onChangeWithValue = (value: number | null) => {
    onChange({
      storeKeys,
      scopes: ['value'],
      type: 'set',
      schema,
      required,
      data: { value },
    });
  };
  const handleEmptyValue = () => {
    onChange({
      storeKeys,
      scopes: ['value'],
      type: 'set',
      schema,
      required,
      data: {
        value: undefined,
      },
    });
  };

  const handleUnitValue = (floatValue: math.MathNumericType) => {
    const valueInBaseUnit2 = math.round(
      math.unit(floatValue, unit).toNumber(initialUnit),
      3,
    ); // Convert value to base unit (e.g. m)
    onChangeWithValue(valueInBaseUnit2);
  };

  const handleNormalValue = (floatValue: number) => {
    onChangeWithValue(floatValue);
  };

  const handleChange = (e: { target: { value: any; }; }) => {
    const val = e.target.value;
    const isLastCharDot = val.slice(-1) === '.';
    const isValidNumber = /^-?\d+(\.\d*)?$/.test(val);

    if (isLastCharDot || isValidNumber || val === '') {
      setValue(val); // Update value with the current value
    } else {
      return;
    }

    if (val === '') {
      handleEmptyValue();
    } else if (includesUnits) {
      handleUnitValue(parseFloat(val));
    } else {
      handleNormalValue(parseFloat(val));
    }
  };

  function isWarning() {
    if (required && (value === undefined || value === '')) {
      return true;
    }
    return false;
  }

  return (
    <>
      <Tooltip title={schema.get('description') as string} placement="top" disableInteractive>
        <TextField
          label={hideTitle ? undefined : <TransTitle schema={schema} storeKeys={storeKeys} />}
          aria-label={hideTitle ? <TransTitle schema={schema} storeKeys={storeKeys} /> as unknown as string : undefined}
          // changing `type` to `text`, to be able to change invalid data
          type="string"
          disabled={schema.get('readOnly') as boolean | undefined}
          multiline={false}
          required={required}
          error={isWarning()}
          minRows={1}
          maxRows={1}
          inputRef={inputRef}
          fullWidth
          variant={schema.getIn(['view', 'variant']) as any}
          margin={schema.getIn(['view', 'margin']) as InputProps['margin']}
          size={schema.getIn(['view', 'dense']) ? 'small' : 'medium'}
          value={value}
          onClick={onClick}
          onFocus={onFocus}
          onBlur={onBlur}
          onKeyUp={onKeyUp}
          id={`uis-${uid}`}
          style={getStyle()}
          onKeyDown={(e) => {
            if (onKeyDown) { onKeyDown(e); }
          }}
          onChange={handleChange}
          InputLabelProps={{ shrink: schema.getIn(['view', 'shrink']) as boolean }}
          InputProps={InputProps}
          // eslint-disable-next-line react/jsx-no-duplicate-props
          inputProps={newInputProps}
        />
      </Tooltip>

      {includesUnits
        ? (
          <FormControl style={{ width: '20%', marginLeft: '0.5rem' }}>
            <InputLabel id="unit-select-label">Unit</InputLabel>
            <Select
              labelId="unit-select-label"
              id="unit-select"
              value={unit}
              onChange={handleUnitChange}
            >
              {(AllUnits && initialDimension && initialUnit)
                ? AllUnits[initialDimension].all.slice(AllUnits[initialDimension].all.indexOf(initialUnit))
                  .map((mapunit: string) => (
                    <MenuItem key={mapunit + uid} value={mapunit}>
                      {mapunit}
                    </MenuItem>
                  )) : null}
            </Select>
          </FormControl>
        )
        : null}

      {isWarning() ? <h2 className="RequiredText">Required</h2>
        : null}
    </>
  );
}
