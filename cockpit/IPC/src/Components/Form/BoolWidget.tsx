/* eslint-disable import/prefer-default-export */
import React, { useEffect, useState } from 'react';
import { FormControlLabel, Switch, Tooltip } from '@mui/material';
import {
  TransTitle, WidgetProps, WithValue, useUIStore,
} from '@ui-schema/ui-schema';
import { MuiWidgetBinding } from '@ui-schema/ds-material/widgetsBinding';
import { useUID } from 'react-uid';

export function BooleanWidget<P extends WidgetProps<MuiWidgetBinding> = WidgetProps<MuiWidgetBinding>>({
  storeKeys, schema, onChange,
}: P & WithValue): React.ReactElement {
  const uid = useUID();
  const { store } = useUIStore();

  function getNestedValue(obj: any, path: any) {
    return path.reduce((xs: any, x: any) => (xs && xs[x] ? xs[x] : false), obj);
  }

  const constValue = schema.get('const') as string;
  const isConst = constValue !== undefined;
  const [value, setValue] = useState<boolean>(isConst ? constValue : getNestedValue(store?.toJS().values, storeKeys.toJS()));

  useEffect(() => {
    if (isConst) {
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
  }, [store]);

  const handleToggle = (event: React.ChangeEvent<HTMLInputElement>) => {
    setValue(event.target.checked);
    onChange({
      storeKeys,
      scopes: ['value'],
      type: 'set',
      schema,
      required: schema.get('required') as boolean,
      data: { value: event.target.checked },
    });
  };

  const description = schema.get('description') as string;

  return (
  // eslint-disable-next-line react/jsx-no-useless-fragment
    <Tooltip title={description || ''}>
      <FormControlLabel
        control={(
          <Switch
            checked={value}
            onChange={handleToggle}
            color="primary"
            disabled={schema.get('readOnly') as boolean || isConst}
            id={`uis-${uid}`}
          />
        )}
        label={schema.get('title') as string ?? <TransTitle schema={schema} storeKeys={storeKeys} />}
      />
    </Tooltip>
  );
}
