/* eslint-disable react/require-default-props */
import React, {
  CSSProperties, useState, useRef,
} from 'react';
import { InputProps } from '@mui/material/Input';
import {
  PluginStack,
  WidgetProps, WithValue, useUIStore,
} from '@ui-schema/ui-schema';
import { MuiWidgetBinding } from '@ui-schema/ds-material/widgetsBinding';
// eslint-disable-next-line import/no-extraneous-dependencies
import { useUID } from 'react-uid';
import './UnitsWidget.css';
import {
  FormControl, InputLabel, MenuItem, Select, SelectChangeEvent,
} from '@mui/material';

export interface VariantWidgetBaseProps {
  style?: CSSProperties
  inputProps?: InputProps['inputProps']
  inputRef?: any
}

export function VariantWidget<P extends WidgetProps<MuiWidgetBinding> = WidgetProps<MuiWidgetBinding>>({
  storeKeys, schema, onChange,
  style, inputProps = {}, inputRef: customInputRef, level,
}: P & VariantWidgetBaseProps & WithValue): React.ReactElement {
  // const { addAlert } = useAlertContext();
  const uid = useUID();
  const { store } = useUIStore();
  // eslint-disable-next-line react-hooks/rules-of-hooks
  const inputRef = customInputRef || useRef();

  // eslint-disable-next-line react-hooks/rules-of-hooks
  // const inputRef = customInputRef || React.useRef();

  console.log('VariantSchema: ', schema.toJS());
  const storeValues = store!.toJS().values || {};
  function getNestedValue(obj: any, keys: Array<any>): any {
    let currentValue = obj;
    for (let i = 0; i < keys.length; i += 1) {
      const key = keys[i];
      if (currentValue[key] === undefined) {
        // If the key doesn't exist, determine what default value to set based on the next key
        if (i < keys.length - 1) {
          currentValue[key] = (typeof keys[i + 1] === 'number') ? [] : {};
        } else {
          // If it's the last key, set the value to undefined
          currentValue[key] = undefined;
        }
      }
      currentValue = currentValue[key];
    }
    return currentValue;
  }

  /**
   *  Find selected object based on data in store.
   * @param oneOf Schema
   * @param vals  Store
   * @returns selected object title
   */
  function findSelectedTitle(oneOf:any, vals:any) {
    if (!vals) { return null; }
    // Get key from store to determine what is selected
    const selectedProp = Object.keys(vals).length > 0 ? Object.keys(vals)[0] : undefined;
    if (!selectedProp) { return null; }

    // if schema is invalid
    if (!oneOf || oneOf.toJS().length === 0) {
      console.log('FAILURE', oneOf.toJS(), vals);
      return null;
    }
    const oneOfJS = oneOf.toJS();

    // Look through each oneOf object to find key under properties.
    for (let i = 0; i < oneOfJS.length; i += 1) {
      if (oneOfJS[i].properties && Object.keys(oneOfJS[i].properties).includes(selectedProp)) {
        return oneOfJS[i].title;
      }
    }
    return null;
  }
  const required = schema.toJS()['x-tfc'] ? schema.toJS()['x-tfc'].required : false;
  const oneOfSchema = schema.get('oneOf');

  const storeValue = getNestedValue(storeValues, storeKeys.toJS());
  console.log('storeValuez: ', storeValue);
  console.log('findSelectedTitle: ', findSelectedTitle(oneOfSchema, storeValue));
  const [selectedTitle, setSelectedTitle] = useState<string | null>(findSelectedTitle(oneOfSchema, storeValue));

  const handleSelectChange = (event: SelectChangeEvent) => {
    onChange({
      storeKeys,
      scopes: ['value'],
      type: 'set',
      schema,
      required,
      data: { },
    });
    setSelectedTitle(event.target.value as string);
  };

  const renderSelectedObject = (title: string | null) => {
    const selectedObject = oneOfSchema.find((item: any) => item.get('title') === title);
    // You can render the selected object here
    // For demonstration, I'm just converting it to JSON string
    return (
      <PluginStack
        schema={selectedObject}
        parentSchema={schema}
        storeKeys={storeKeys}
        level={level + 1}
      />
    );
    return <pre>{JSON.stringify(selectedObject, null, 2)}</pre>;
  };

  const textStyle = {
    ...style,
    width: '100%',
  };

  return (
    <>
      <FormControl style={{ width: '100%', marginBottom: '1.2rem' }}>
        <InputLabel>Choose Variant</InputLabel>
        <Select
          value={selectedTitle || ''}
          onChange={handleSelectChange}
          type="string"
          disabled={schema.get('readOnly') as boolean | undefined}
          multiline={false}
          required={required}
          error={required && (selectedTitle === undefined || selectedTitle === '')}
          minRows={1}
          maxRows={1}
          inputRef={inputRef}
          fullWidth
          variant={schema.getIn(['view', 'variant']) as any}
          margin={schema.getIn(['view', 'margin']) as InputProps['margin']}
          size={schema.getIn(['view', 'dense']) ? 'small' : 'medium'}
          id={`uis-${uid}`}
          style={textStyle}
          // eslint-disable-next-line react/jsx-no-duplicate-props
          inputProps={inputProps}
        >
          {oneOfSchema.map((item: any) => (
            <MenuItem key={item.get('title')} value={item.get('title')}>
              {item.get('title')}
            </MenuItem>
          ))}
        </Select>
      </FormControl>

      {renderSelectedObject(selectedTitle)}

      {/* <Tooltip title={schema.get('description') as string} placement="top" disableInteractive>
        <TextField
          label={hideTitle ? undefined : <TransTitle schema={schema} storeKeys={storeKeys} />}
          aria-label={hideTitle ? <TransTitle schema={schema} storeKeys={storeKeys} /> as unknown as string : undefined}
          // changing `type` to `text`, to be able to change invalid data
          type="string"
          disabled={schema.get('readOnly') as boolean | undefined}
          multiline={false}
          required={required}
          error={required && (value === undefined || value === '')}
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
          onChange={handleChange}
          InputLabelProps={{ shrink: schema.getIn(['view', 'shrink']) as boolean }}
          InputProps={InputProps}
          // eslint-disable-next-line react/jsx-no-duplicate-props
          inputProps={newInputProps}
        />
      </Tooltip> */}

      {/* {required && (value === undefined || value === '')
        ? <h2 className="RequiredText">Required</h2>
        : null} */}
    </>
  );
}
