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

  useEffect(() => {
    if (!isConst) {
      setValue(getNestedValue(store?.toJS().values, storeKeys.toJS()));
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

  return (
    <Tooltip title={description || ''}>
      <TextField
        value={value}
        onChange={handleTextChange}
        label={schema.get('title') as string ?? <TransTitle schema={schema} storeKeys={storeKeys} />}
        id={`uis-${uid}`}
        fullWidth
        margin="normal"
        InputProps={{
          readOnly: isConst,
        }}
      />
    </Tooltip>
  );
}
