/* eslint-disable @typescript-eslint/no-unused-vars */
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
import Qty from 'js-quantities';
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

export function UnitWidget<P extends WidgetProps<MuiWidgetBinding> = WidgetProps<MuiWidgetBinding>>({ // NOSONAR
  storeKeys, schema, onChange,
  style, onClick, onFocus, onBlur, onKeyUp, onKeyDown,
  // eslint-disable-next-line @typescript-eslint/no-shadow
  inputProps = {}, InputProps = {}, inputRef: customInputRef,
  steps = 'any',
}: P & UnitWidgetBaseProps & WithValue): React.ReactElement {
  const uid = useUID();
  const { store } = useUIStore();

  let initialDimension: string | undefined;
  const storeValues = store ? store.toJS().values : {};
  let storeValue = getNestedValue(storeValues, storeKeys.toJS());
  initialDimension = schema.toJS()['x-tfc'] ? schema.toJS()['x-tfc'].dimension : undefined;
  const type = schema.get('type') as string | undefined;
  const initialUnit = schema.toJS()['x-tfc'] ? schema.toJS()['x-tfc'].unit : undefined;
  const required = schema.toJS()['x-tfc'] ? schema.toJS()['x-tfc'].required : false;
  const { minimum, maximum } = schema.toJS();
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

  if (typeof storeValue === 'object') {
    storeValue = undefined;
  }

  const [unit, setUnit] = React.useState<string>(initialUnit ?? '');
  const [value, setValue] = React.useState<Qty | undefined>(
    storeValue !== undefined
      ? Qty(`${storeValue}${initialUnit ?? ''}`)
      : undefined,
  );
  const [errText, setErrText] = React.useState('');
  const min = Qty(`${minimum}${initialUnit ?? ''}`);
  const max = Qty(`${maximum}${initialUnit ?? ''}`);
  // eslint-disable-next-line no-param-reassign, @typescript-eslint/no-unused-vars

  const handleUnitChange = (event: any) => {
    const newUnit = event.target.value;
    if (value !== null && value !== undefined && newUnit && initialUnit) {
      setValue(value.to(newUnit).toPrec(initialUnit));
      setUnit(newUnit);
      onChange({
        storeKeys,
        scopes: ['value'],
        type: 'set',
        schema,
        required,
        data: {
          value:
            value.to(initialUnit).toPrec(initialUnit).scalar,
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
      width: !value?.isUnitless() ? 'calc(80% - 0.5rem)' : '100%',
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
        value: { internal_null_value_do_not_use: null },
      },
    });
  };

  const handleUnitValue = (floatValue: math.MathNumericType) => {
    if (floatValue === null || floatValue === undefined || typeof floatValue !== 'number') {
      return;
    }
    let valueInBaseUnit = floatValue;
    if (value && !value.isUnitless()) {
      valueInBaseUnit = Qty(`${floatValue}${unit}`).to(initialUnit).toPrec(initialUnit).scalar;
    }
    setValue(Qty(`${floatValue}${unit}`));
    onChangeWithValue(valueInBaseUnit);
  };

  const [addComma, setAddComma] = React.useState<boolean>(false);

  const handleChange = (e: any) => {
    const val = e.target.value;
    const isLastCharDot = val.slice(-1) === '.';
    const isValidNumber = /^-?\d+(\.\d*)?$/.test(val);
    console.log('handleChange', val, isValidNumber, isLastCharDot, value?.isUnitless());
    if (!isValidNumber && val !== '') {
      return;
    }
    if (type === 'integer' && e.nativeEvent.data === '.' && value?.isUnitless()) {
      return; // Don't allow decimal points in integer fields (unless there is a unit)
    }
    if (isLastCharDot && isValidNumber && e.nativeEvent.data === '.') {
      setAddComma(true);
      return;
    }
    setAddComma(false);
    if (val === '') {
      setValue(undefined);
      handleEmptyValue();
    } else {
      handleUnitValue(parseFloat(val));
    }
  };

  function isWarning() {
    if (required && (!value || value.toString() === '')) {
      if (errText !== 'Required') setErrText('Required');
      return true;
    }
    if (!value || !min || !max) {
      return false;
    }

    if (min.compareTo(value) === 1) {
      if (errText !== `Minimum value: ${min.toString()}`) setErrText(`Minimum value: ${min.toString()}`);
      return true;
    }
    if (max.compareTo(value) === -1) {
      if (errText !== `Maxiumum value: ${max.toString()}`) setErrText(`Maxiumum value: ${max.toString()}`);
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
          autoComplete="off"
          inputRef={inputRef}
          fullWidth
          variant={schema.getIn(['view', 'variant']) as any}
          margin={schema.getIn(['view', 'margin']) as InputProps['margin']}
          size={schema.getIn(['view', 'dense']) ? 'small' : 'medium'}
          value={`${value ? value.scalar.toString() : ''}${addComma ? '.' : ''}`}
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

      {(value && !value.isUnitless())
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

      {isWarning() ? <h2 className="RequiredText">{errText}</h2>
        : null}
    </>
  );
}
