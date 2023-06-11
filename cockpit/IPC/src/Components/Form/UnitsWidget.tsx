/* eslint-disable prefer-const */
/* eslint-disable no-continue */
/* eslint-disable react/require-default-props */
import React, {
  CSSProperties, FocusEventHandler, KeyboardEventHandler, MouseEventHandler,
} from 'react';
import TextField from '@mui/material/TextField';
import InputAdornment from '@mui/material/InputAdornment';
import { InputProps } from '@mui/material/Input';
import {
  FormControl, InputLabel, MenuItem, Select, Tooltip,
} from '@mui/material';
// eslint-disable-next-line import/no-extraneous-dependencies
import { useUID } from 'react-uid';
import { TransTitle } from '@ui-schema/ui-schema/Translate/TransTitle';
import { mapSchema } from '@ui-schema/ui-schema/Utils/schemaToNative';
import { schemaTypeIs } from '@ui-schema/ui-schema/Utils/schemaTypeIs';
import {
  StoreKeys, UIStoreType, WidgetProps, WithValue, useUIStore,
} from '@ui-schema/ui-schema';
import { MuiWidgetBinding } from '@ui-schema/ds-material/widgetsBinding';

import * as math from 'mathjs';
import { AlertVariant } from '@patternfly/react-core';
import { Units } from './units';
import { useAlertContext } from '../Alert/AlertContext';

export interface UnitWidgetBaseProps {
  style?: CSSProperties
  onClick?: MouseEventHandler<HTMLDivElement> | undefined
  onFocus?: FocusEventHandler<HTMLInputElement | HTMLTextAreaElement> | undefined
  onBlur?: FocusEventHandler<HTMLInputElement | HTMLTextAreaElement> | undefined
  onKeyUp?: KeyboardEventHandler<HTMLDivElement> | undefined
  onKeyDown?: KeyboardEventHandler<HTMLDivElement> | undefined
  steps?: number | 'any'
  inputProps?: InputProps['inputProps']
  InputProps?: Partial<InputProps>
  inputRef?: any
}

interface StoreType {
  unit: string
  value: string
  dimension: string
}

function updateStore(
  store: UIStoreType<any> | undefined,
  storeKeys: StoreKeys,
  updatedStoreValue: { value: number; unit?: string; dimension?: string | undefined; },
  includesUnits: boolean,
) {
  // First, we make a deep copy of the store
  const newStore = JSON.parse(JSON.stringify(store)).values;

  const keyList = storeKeys.toJS();
  const parentObject = newStore;
  keyList.reduce((object: any, key: any, index) => {
    if (key === 'config' && !object[key]) { return object; }
    // if we're at the last index, return the updated store value
    if (index === keyList.length - 1) {
      // eslint-disable-next-line no-param-reassign
      object[key] = includesUnits ? updatedStoreValue : updatedStoreValue.value;
    }
    return object[key];
  }, parentObject);

  // Finally, we return the new store object
  console.log('newStore: ', newStore);
  return newStore;
}

export function UnitWidget<P extends WidgetProps<MuiWidgetBinding> = WidgetProps<MuiWidgetBinding>>({
  storeKeys, schema, onChange,
  showValidity, valid, errors, required,
  style,
  onClick, onFocus, onBlur, onKeyUp, onKeyDown,
  // eslint-disable-next-line @typescript-eslint/no-shadow
  inputProps = {}, InputProps = {}, inputRef: customInputRef,
  widgets, steps = 'any',
}: P & UnitWidgetBaseProps & WithValue): React.ReactElement {
  const uid = useUID();

  // eslint-disable-next-line react-hooks/rules-of-hooks
  const inputRef = customInputRef || React.useRef();

  const { store } = useUIStore();

  const { addAlert } = useAlertContext();

  let storeValue: StoreType;
  let initialDimension: string | undefined;
  let includesUnits = true;

  const storeValues = store!.getValues() || {};
  let currentStoreValue = storeValues;
  // eslint-disable-next-line no-restricted-syntax
  for (const key of storeKeys.toJS()) {
    if (currentStoreValue[key] && typeof currentStoreValue[key] === 'object') {
      currentStoreValue = currentStoreValue[key];
    } else if (currentStoreValue[key]) {
      currentStoreValue.value = currentStoreValue[key];
      break;
    } else {
      try {
        currentStoreValue = currentStoreValue.toJS()[key];
      } catch (e) {
        if (key === 'config') { continue; } // Depending on data, config may or may not be the top object
        addAlert('Something went wrong, Key not found in store', AlertVariant.danger);
        console.warn(`Key "${key}" not found in store. ${JSON.stringify(currentStoreValue)}`);
        break;
      }
    }
  }
  storeValue = currentStoreValue;
  initialDimension = storeValue.dimension || undefined;
  if (!initialDimension) {
    includesUnits = false;
  }

  // eslint-disable-next-line no-param-reassign
  inputProps = mapSchema(inputProps, schema);
  const AllUnits = Units as { [key: string]: { all: string[], default: string } };

  const initialUnit = storeValue.unit || 'NoUnit';
  // Maybe we could default to SI units using .toSI().toJSON() (properties: unit, value)
  // Could make it easier to configure.
  // Hz becomes s^-1, m/s becomes m*s^-1, etc. (https://mathjs.org/docs/datatypes/units.html)
  // We might want to override this to get Hz instead of s^-1, etc.

  const [unit, setUnit] = React.useState(initialUnit);
  const [value, setValue] = React.useState<string>((storeValue && storeValue.value) ? storeValue.value : '0');

  // empty storeKeys
  const rootStore = [] as unknown as StoreKeys;

  const handleUnitChange = (event: any) => {
    const newUnit = event.target.value;
    if (value !== null && newUnit && initialUnit) {
      const floatValue = parseFloat(value);
      const valueueInBaseUnit = math.unit(floatValue, unit).toNumber(initialUnit); // Convert current value to base unit (e.g. m)
      const newValueInNewUnit = math.round(math.unit(valueueInBaseUnit, initialUnit).toNumber(newUnit), 3); // Convert value to new unit
      setValue(newValueInNewUnit.toString());
      setUnit(newUnit);

      onChange({
        storeKeys: rootStore,
        scopes: ['value'],
        type: 'set',
        schema,
        required,
        data: {
          value: updateStore(
            store,
            storeKeys,
            { value: valueueInBaseUnit, unit: initialUnit, dimension: initialDimension },
            true,
          ),
        },
      });
    }
  };
  const hideTitle = schema.getIn(['view', 'hideTitle']);
  const InfoRenderer = widgets?.InfoRenderer;
  if (InfoRenderer && schema?.get('info')) {
    // eslint-disable-next-line no-param-reassign
    InputProps.endAdornment = (
      <InputAdornment position="end">
        <InfoRenderer
          schema={schema}
          variant="icon"
          openAs="modal"
          storeKeys={storeKeys}
          valid={valid}
          errors={errors}
        />
      </InputAdornment>
    );
  }

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

  const textStyle = {
    ...style,
    width: includesUnits ? 'calc(80% - 0.5rem)' : '100%',
  };

  return (
    <>
      <Tooltip title={schema.get('description') as string} placement="top">
        <TextField
          label={hideTitle ? undefined : <TransTitle schema={schema} storeKeys={storeKeys} />}
          aria-label={hideTitle ? <TransTitle schema={schema} storeKeys={storeKeys} /> as unknown as string : undefined}
          // changing `type` to `text`, to be able to change invalid data
          type="string"
          disabled={schema.get('readOnly') as boolean | undefined}
          multiline={false}
          required={required}
          error={!valid && showValidity && false}
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
          style={textStyle}
          onKeyDown={(e) => {
            if (onKeyDown) { onKeyDown(e); }
          }}
          onChange={(e) => {
            const val = e.target.value;
            if (val.slice(-1) === '.') {
              setValue(val);
              return;
            }
            if (!/^-?\d+(\.\d*)?$/.test(val)) {
              return;
            }
            const floatValue = parseFloat(val);
            setValue(val); // Update value with the current value

            if (includesUnits) {
              const valueueInBaseUnit2 = math.round(
                math.unit(floatValue, unit).toNumber(initialUnit),
                3,
              ); // Convert current value to base unit
              onChange({
                storeKeys: rootStore,
                scopes: ['value'],
                type: 'set',
                schema,
                required,
                data: {
                  value: updateStore(
                    store,
                    storeKeys,
                    { value: valueueInBaseUnit2, unit: initialUnit, dimension: initialDimension },
                    true,
                  ),
                },
              });
            } else {
              onChange({
                storeKeys: rootStore,
                scopes: ['value'],
                type: 'set',
                schema,
                required,
                // eslint-disable-next-line max-len
                data: { value: updateStore(store, storeKeys, { value: floatValue }, false) },
              });
            }
          }}
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
              {(AllUnits && initialDimension) ? AllUnits[initialDimension].all.map((mapunit: string, index: number) => (
                // eslint-disable-next-line react/no-array-index-key
                <MenuItem key={mapunit + uid + index} value={mapunit}>
                  {mapunit}
                </MenuItem>
              )) : null}
            </Select>
          </FormControl>
        )
        : null}

      {/* <ValidityHelperText
        errors={errors}
        showValidity={showValidity}
        schema={schema}
      /> */}
    </>
  );
}
