/* eslint-disable import/prefer-default-export */
import React, { useEffect, useState } from 'react';
import { TextField, Tooltip } from '@mui/material';
import {
  TransTitle, WidgetProps, WithValue, useUIStore,
} from '@ui-schema/ui-schema';
import { MuiWidgetBinding } from '@ui-schema/ds-material/widgetsBinding';
import { useUID } from 'react-uid';

export function StringWidget<P extends WidgetProps<MuiWidgetBinding> = WidgetProps<MuiWidgetBinding>>({
  storeKeys, schema, onChange,
}: P & WithValue): React.ReactElement {
  const uid = useUID();
  const { store } = useUIStore();

  function getNestedValue(obj: any, path: any) {
    return path.reduce((xs: any, x: any) => (xs && xs[x] !== undefined ? xs[x] : ''), obj);
  }
  const constValue = schema.get('const') as string;
  const isConst = constValue !== undefined;
  const [value, setValue] = useState<string>(isConst ? constValue : getNestedValue(store?.toJS().values, storeKeys.toJS()));
  console.log(schema.toJS());
  useEffect(() => {
    if (!isConst) {
      setValue(getNestedValue(store?.toJS().values, storeKeys.toJS()));
    } else {
      onChange({
        storeKeys,
        scopes: ['value'],
        type: 'set',
        schema,
        required: schema.get('required') as boolean,
        data: { value: constValue },
      });
    }
  }, [store, storeKeys, isConst]);

  const handleTextChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const newValue = event.target.value;
    setValue(newValue);
    onChange({
      storeKeys,
      scopes: ['value'],
      type: 'set',
      schema,
      required: schema.get('required') as boolean,
      data: { value: newValue },
    });
  };

  const description = schema.get('description') as string;
  const maxLength = schema.get('maxLength') as number ?? undefined;
  const minLength = schema.get('minLength') as number ?? undefined;
  const pattern = schema.get('pattern') as string ?? undefined;

  function getHelperText(): string {
    if (value.length < minLength) {
      return `Length must be at least ${minLength}`;
    }
    if (value.length > maxLength) {
      return `Length must be less than ${maxLength}`;
    }
    if (!RegExp(pattern).test(value)) {
      return 'Invalid input';
    }
    return '';
  }

  return (
    <Tooltip title={description || ''}>
      <TextField
        value={value}
        onChange={handleTextChange}
        label={schema.get('title') as string ?? <TransTitle schema={schema} storeKeys={storeKeys} />}
        id={`uis-${uid}`}
        fullWidth
        error={value.length < minLength || value.length > maxLength || !RegExp(pattern).test(value)}
        helperText={value.length < minLength || value.length > maxLength || !RegExp(pattern).test(value) ? getHelperText() : undefined}
        margin="normal"
        InputProps={{
          readOnly: schema.get('readOnly') as boolean || isConst,
        }}
      />
    </Tooltip>
  );
}
