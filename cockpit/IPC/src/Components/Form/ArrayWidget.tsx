/* eslint-disable import/prefer-default-export */
import React, { useEffect, useState } from 'react';
import {
  Box, FormControl, FormLabel, Grid, IconButton, Tooltip,
} from '@mui/material';
import DeleteIcon from '@mui/icons-material/Delete';
import {
  PluginStack,
  StoreKeys,
  StoreSchemaType,
  TransTitle, WidgetProps, WithValue, useUIStore,
} from '@ui-schema/ui-schema';
import { uid } from 'react-uid';
import { MuiWidgetBinding } from '@ui-schema/ds-material/widgetsBinding';
import { PlusIcon } from '@patternfly/react-icons';
import { useDarkMode } from '../Simple/DarkModeContext';

export function ArrayWidget<P extends WidgetProps<MuiWidgetBinding> = WidgetProps<MuiWidgetBinding>>({
  storeKeys, schema, onChange, widgets, valid, errors, required, showValidity, level,
}: P & WithValue): React.ReactElement {
  const { store } = useUIStore();

  function getNestedValue(obj: any, path: any) {
    return path.reduce((xs: any, x: any) => (xs && xs[x] !== undefined ? xs[x] : ''), obj);
  }
  const [value, setValue] = useState<any[]>(getNestedValue(store?.toJS().values, storeKeys.toJS()));
  //   const notSortable = schema.get('notSortable') as boolean | undefined;
  //   const notAddable = schema.get('notAddable') as boolean | undefined;
  //   const notDeletable = schema.get('notDeletable') as boolean | undefined;
  const maxItems = schema.get('maxItems') as number | undefined;
  const minItems = schema.get('minItems') as number | undefined;
  const title = schema.get('title') as string | undefined;
  const InfoRenderer = widgets?.InfoRenderer;
  const { isDark } = useDarkMode();

  useEffect(() => {
    if (store) {
      setValue(getNestedValue(store?.toJS().values, storeKeys.toJS()));
    }
  }, [store, storeKeys]);

  // eslint-disable-next-line @typescript-eslint/no-unused-vars
  const handleRemove = (index: number) => {
    if (minItems && value.length <= minItems) {
      return;
    }
    const newValue = [...value];
    newValue.splice(index, 1);
    setValue(newValue);
    onChange({
      storeKeys,
      scopes: ['value', 'internal'],
      type: 'list-item-delete',
      index,
      schema,
      required: schema.get('required') as boolean,
    });
  };

  const handleAdd = () => {
    if (maxItems && value.length >= maxItems) {
      return;
    }
    const newValue = [...value];
    newValue.push({});
    setValue(newValue);
    onChange({
      storeKeys,
      scopes: ['value', 'internal'],
      type: 'list-item-add',
      schema,
      required: schema.get('required') as boolean,
    });
  };

  const info = InfoRenderer && schema?.get('info')
    ? (
      <InfoRenderer
        schema={schema}
        variant="preview"
        openAs="embed"
        storeKeys={storeKeys}
        valid={valid}
        errors={errors}
      />
    ) : null;

  const itemStyle = {
    backgroundColor: isDark ? '#1A1A1A' : '#F9F9F9',
    padding: '1rem',
    marginBottom: '1rem',
    display: 'flex',
    justifyContent: 'space-between',
    alignItems: 'flex-start',
    borderRadius: '0.5rem',
    width: '100%',
  };

  return (
    <FormControl required={required} error={!valid && showValidity} component="fieldset" style={{ width: '100%' }}>
      {!schema.getIn(['view', 'hideTitle'])
        ? (
          <Box mb={1}>
            <Box mb={1}>
              <FormLabel component="legend">
                {title ?? <TransTitle schema={schema} storeKeys={storeKeys} />}
              </FormLabel>
            </Box>

            {info}
          </Box>
        ) : null}

      {schema.getIn(['view', 'hideTitle'])
        ? <Box mb={1}>{info}</Box> : null}

      <Grid container spacing={3} style={{ margin: '0rem' }}>
        {value?.map((_, i) => (
          <Box key={`${uid(i)}-${i + 1}`} style={itemStyle}>
            <Box style={{ marginRight: '1rem', marginLeft: '0.5rem' }} key={`Box-${i + 1}`}>{i}</Box>
            <div
              style={{
                width: '100%',
                display: 'flex',
                flexDirection: 'column',
                marginTop: '1.2rem',
              }}
              key={`div-${i + 1}`}
            >
              <PluginStack<{ schemaKeys: StoreKeys | undefined }>
                showValidity={showValidity}
                schema={schema.get('items') as StoreSchemaType}
                parentSchema={schema}
                storeKeys={storeKeys.push(i)}
                key={`${i + 1}-PluginStack`}
                level={level + 1}
                schemaKeys={storeKeys?.push('items')}
              />
            </div>
            <Tooltip title="Remove Item">
              <IconButton onClick={() => handleRemove(i)} style={{ marginLeft: '1rem' }}>
                <DeleteIcon />
              </IconButton>
            </Tooltip>
          </Box>
        ))}
        <Box style={{ width: '100%', textAlign: 'center' }}>
          <Tooltip title="Add Item">
            <IconButton
              disabled={(maxItems && value.length >= maxItems) as boolean}
              onClick={handleAdd}
            >
              <PlusIcon />
            </IconButton>
          </Tooltip>
        </Box>
      </Grid>
    </FormControl>
  );
}
