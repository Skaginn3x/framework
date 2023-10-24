/* eslint-disable @typescript-eslint/no-shadow */
/* eslint-disable @typescript-eslint/no-unused-vars */
/* eslint-disable prefer-const */
/* eslint-disable no-continue */
/* eslint-disable react/require-default-props */
import React, {
  CSSProperties, FocusEventHandler, KeyboardEventHandler, MouseEventHandler,
} from 'react';
import TextField from '@mui/material/TextField';
import { InputProps } from '@mui/material/Input';
import { Tooltip } from '@mui/material';
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
import UnitsDropdown from './UnitsDropdown';
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

  let dimension: string | undefined;
  const storeValues = store ? store.toJS().values : {};
  let storeValue = getNestedValue(storeValues, storeKeys.toJS());
  dimension = schema.toJS()['x-tfc']?.dimension ? schema.toJS()['x-tfc'].dimension : undefined;
  const type = schema.get('type') as string | undefined;
  let initialUnit = schema.toJS()['x-tfc']?.unit.unit_ascii ? schema.toJS()['x-tfc'].unit.unit_ascii : undefined;
  const required = schema.toJS()['x-tfc']?.required ? schema.toJS()['x-tfc'].required : false;
  const { minimum, maximum } = schema.toJS();
  let inputRef = React.useRef();
  if (customInputRef) {
    inputRef = customInputRef;
  }

  // eslint-disable-next-line no-param-reassign
  inputProps = mapSchema(inputProps, schema);

  if (initialUnit === '%' && dimension === 'ratio') {
    // eslint-disable-next-line no-param-reassign
    InputProps = {
      ...InputProps,
      endAdornment: '%',
    };
    initialUnit = undefined;
    dimension = undefined;
  }

  if (typeof storeValue === 'object') {
    storeValue = undefined;
  }

  const [unit, setUnit] = React.useState<string>(initialUnit ?? '');
  const [stringValue, setStringValue] = React.useState<string>(storeValue?.toString() ?? '');
  const [value, setValue] = React.useState<Qty | undefined>(
    storeValue !== undefined
      ? Qty(`${storeValue}${initialUnit ?? ''}`)
      : undefined,
  );
  const [errText, setErrText] = React.useState('');
  const min = Qty(`${minimum}${initialUnit ?? ''}`);
  const max = Qty(`${maximum}${initialUnit ?? ''}`);

  /**
   * Handles the change of the unit in the UnitsWidget component.
   * @param event - The event object.
   */
  const handleUnitChange = (event: any) => {
    const newUnit = event.target.value;
    console.log(value);
    console.log(newUnit);
    if (value !== null && value !== undefined && newUnit && initialUnit) {
      setValue(value.to(newUnit).toPrec(initialUnit));
      setStringValue(value.to(newUnit).toPrec(initialUnit).scalar.toString());
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
    } else if (newUnit) {
      setUnit(newUnit);
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
      width: initialUnit && dimension ? 'calc(80% - 0.5rem)' : '100%',
    };
  }

  /**
   * Handles the change of the value in the UnitsWidget component.
   * @param value - The value to be changed.
  */
  const onChangeWithValue = (value: number) => {
    onChange({
      storeKeys,
      scopes: ['value'],
      type: 'set',
      schema,
      required,
      data: { value },
    });
  };

  /**
   * Handles null value in the UnitsWidget component.
   * This is because UI-Schema does weird things to nulls.
   */
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

  /**
   * Handles the change of the value in the UnitsWidget component.
   * @param e - The event object.
   * @returns Nothing.
  */
  const handleChange = (e: any) => {
    const val = e.target.value as string;
    // if there is already a decimal point, ignore any more
    if (val && e.nativeEvent.data === '.' && val.slice(0, -1).indexOf('.') !== -1) {
      return;
    }
    if (e.nativeEvent.data && !e.nativeEvent.data.match(/[0-9.]/)) {
      // If anything other than a number or a decimal point is entered, ignore it.
      return;
    }

    setStringValue(val);

    if (val === '') {
      handleEmptyValue();
      return;
    }
    if (parseFloat(val) === undefined || Number.isNaN(parseFloat(val))) {
      setErrText('Invalid number');
      setValue(undefined);
      return;
    }
    handleUnitValue(parseFloat(val));
  };

  /**
   * Checks if the value is invalid.
   * @returns true if the value is invalid, false otherwise.
   */
  function isWarning() { // NOSONAR
    if (required && (!value || value.toString() === '')) {
      if (errText !== 'Required') setErrText('Required');
      return true;
    }

    if (type === 'integer' && value?.isUnitless() && value?.scalar.toString().indexOf('.') !== -1) {
      if (errText !== 'Must be an integer') setErrText('Must be an integer');
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
          value={stringValue ?? ''}
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

      <UnitsDropdown
        initialDimension={dimension}
        key={`${dimension}-${uid}`}
        initialUnit={initialUnit}
        handleUnitChange={handleUnitChange}
        unit={unit}
      />

      {isWarning() ? <h2 className="RequiredText">{errText}</h2>
        : null}
    </>
  );
}
