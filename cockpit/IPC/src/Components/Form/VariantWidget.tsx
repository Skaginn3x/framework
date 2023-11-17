/* eslint-disable react/jsx-one-expression-per-line */
/* eslint-disable react/require-default-props */
import React, {
  CSSProperties, useState,
} from 'react';
import { InputProps } from '@mui/material/Input';
import {
  FormControl, InputLabel, MenuItem, Select, SelectChangeEvent,
} from '@mui/material';
import {
  PluginStack,
  WidgetProps, WithValue, useUIStore,
} from '@ui-schema/ui-schema';
import { MuiWidgetBinding } from '@ui-schema/ds-material/widgetsBinding';
import { useUID } from 'react-uid';
import './UnitsWidget.css';
import { getNestedValue } from './WidgetFunctions';

export interface VariantWidgetBaseProps {
  style?: CSSProperties
  inputProps?: InputProps['inputProps']
  inputRef?: any
}

export function VariantWidget<P extends WidgetProps<MuiWidgetBinding> = WidgetProps<MuiWidgetBinding>>({
  storeKeys, schema, onChange,
  style, inputProps = {}, inputRef: customInputRef, level,
}: P & VariantWidgetBaseProps & WithValue): React.ReactElement {
  const uid = useUID();
  const { store } = useUIStore();
  // VV  Might cause errors, need to look into VV
  let inputRef = React.useRef();
  if (customInputRef) {
    inputRef = customInputRef;
  }

  const storeValues = store!.toJS().values || {};

  function getType(oneOf:any, selected:any): string | string[] | null {
    if (!selected) { return null; }
    // if schema is invalid
    if (!oneOf || oneOf.toJS().length === 0) {
      return null;
    }
    const oneOfJS = oneOf.toJS();

    // Look through each oneOf object to find key under properties.
    // eslint-disable-next-line no-restricted-syntax
    for (const element of oneOfJS) {
      if (element?.type && element.title === selected) {
        return element.type;
      }
    }
    return null;
  }

  /**
   *  Find selected object based on data in store.
   * @param oneOf Schema
   * @param vals  Store
   * @returns selected object title
   */
  function findSelectedTitle(oneOf:any, vals:any) {
    if (vals === undefined) { return null; }

    // if vals is null, return std::monostate
    if (vals === null && oneOf.toJS().map((item: any) => item.title).includes('std::monostate')) {
      return 'std::monostate';
    }
    if (vals === null) { return null; }
    // Get key from store to determine what is selected
    console.log('vals', vals);
    const selectedProp = Object.keys(vals).length > 0 ? Object.keys(vals)[0] : undefined;
    if (!selectedProp) { return null; }

    // if schema is invalid
    if (!oneOf || oneOf.toJS().length === 0) {
      console.error('Cannot find selected object.');
      return null;
    }
    const oneOfJS = oneOf.toJS();

    // Look through each oneOf object to find key under properties.
    // eslint-disable-next-line no-restricted-syntax
    for (const element of oneOfJS) {
      if (element.properties && Object.keys(element.properties).includes(selectedProp)) {
        return element.title;
      }
    }
    return null;
  }

  const required = schema.toJS()['x-tfc'] ? schema.toJS()['x-tfc'].required : false;
  console.log(schema.toJS());
  const oneOfSchema = schema.get('oneOf');

  const storeValue = getNestedValue(storeValues, storeKeys.toJS());
  const [selectedTitle, setSelectedTitle] = useState<string | null>(findSelectedTitle(oneOfSchema, storeValue));

  function findConst(oneOf:any, selected:any) {
    // eslint-disable-next-line no-restricted-syntax
    for (const item of oneOf) {
      if (item.get('title') === selected) {
        return item.get('const');
      }
    }
    return null;
  }

  const handleSelectChange = (event: SelectChangeEvent) => {
    const type = getType(oneOfSchema, event.target.value);

    if ((Array.isArray(type) && type.includes('null')) || (!Array.isArray(type) && type === 'null')) {
      onChange({
        storeKeys,
        scopes: ['value'],
        type: 'set',
        schema,
        required,
        data: { value: { internal_null_value_do_not_use: null } },
      });
    } else {
      onChange({
        storeKeys,
        scopes: ['value'],
        type: 'set',
        schema,
        required,
        data: { value: findConst(oneOfSchema, event.target.value) },
      });
    }
    setSelectedTitle(event.target.value);
  };

  /**
   * Renders the appropriate schema depending on selection
   * @param title Title of selected oneOf object
   * @returns Nested schema render
   */
  const renderSelectedObject = (title: string | null) => {
    const selectedObject = oneOfSchema.find((item: any) => item.get('title') === title);
    console.log('title', title);
    console.log('oneofSchema', oneOfSchema.toJS());
    if (selectedObject && (Object.keys(selectedObject.toJS()).includes('const') || selectedObject.toJS().type === 'null')) {
      console.log('selectedObject', selectedObject.toJS());
      return null;
    }
    return (
      <PluginStack
        schema={selectedObject}
        parentSchema={schema}
        storeKeys={storeKeys}
        level={level + 1}
      />
    );
  };

  const textStyle = {
    ...style,
    width: '100%',
  };

  return (
    <>
      <FormControl style={{ width: '100%', marginBottom: '1.2rem' }}>
        <InputLabel>Choose Variant</InputLabel> {/* Might want to change to actual title */}
        <Select
          value={selectedTitle ?? ''}
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
          variant={schema.getIn(['view', 'variant']) as 'filled' | 'standard' | 'outlined' | undefined}
          margin={schema.getIn(['view', 'margin']) as InputProps['margin']}
          size={schema.getIn(['view', 'dense']) ? 'small' : 'medium'}
          id={`uis-${uid}`}
          style={textStyle}
          inputProps={inputProps}
        >
          {oneOfSchema.map((item: any) => (
            <MenuItem key={item.get('title')} value={item.get('title')}>
              {item.get('title')}
            </MenuItem>
          ))}
        </Select>
      </FormControl>
      { renderSelectedObject(selectedTitle) }

    </>
  );
}
