/* eslint-disable import/prefer-default-export */
import React, { useState } from 'react';
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
    return path.reduce((xs: any, x: any) => (xs[x] ? xs[x] : null), obj);
  }

  const [value, setValue] = useState<boolean>(getNestedValue(store?.toJS().values, storeKeys.toJS()));
  const isConst = schema.get('const') as boolean ?? false;

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
    isConst ? <></>
      : (
        <Tooltip title={description || ''}>
          <FormControlLabel
            control={(
              <Switch
                checked={value}
                onChange={handleToggle}
                color="primary"
                id={`uis-${uid}`}
              />
        )}
            label={schema.get('title') as string ?? <TransTitle schema={schema} storeKeys={storeKeys} />}
          />
        </Tooltip>
      )
  );
}
