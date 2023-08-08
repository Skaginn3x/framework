/* eslint-disable no-param-reassign */
/* eslint-disable @typescript-eslint/no-unused-vars */
import React from 'react';

// "global" ui-config
import { UIMetaProvider } from '@ui-schema/ui-schema/UIMeta';
// for data-stores / data-binding
import {
  UIStoreProvider, WithOnChange, WithValue, createStore,
} from '@ui-schema/ui-schema/UIStore';
import { storeUpdater } from '@ui-schema/ui-schema/storeUpdater';

// for deep immutables
import { createOrderedMap } from '@ui-schema/ui-schema/Utils/createMap';
// for `t` keyword support / basic in-schema translation
import { relTranslator } from '@ui-schema/ui-schema/Translate/relT';

// import the widgets for your design-system.
import { MuiWidgetBinding, widgets } from '@ui-schema/ds-material';
import { injectPluginStack } from '@ui-schema/ui-schema';
import { GridContainer } from '@ui-schema/ds-material/GridContainer';
import Immutable from 'immutable';
import { Button } from '@patternfly/react-core';
import { UnitWidget } from 'src/Components/Form/UnitsWidget';
import { WidgetProps as BaseWidgetProps } from '@ui-schema/ui-schema/Widget';

const GridStack = injectPluginStack(GridContainer);

// export default function FormGenerator(inputSchema: any) {
export default function FormGenerator(
  { inputSchema, onSubmit, values }: { inputSchema: any, onSubmit: (data: any) => void, values: any },
) {
  type JsonType = {
    [key: string]: any;
  };

  interface ExtendedWidgetProps extends BaseWidgetProps<MuiWidgetBinding>, WithOnChange, WithValue { }

  type CustomWidgetBinding = {
    [type: string]: React.FunctionComponent<ExtendedWidgetProps> | React.ComponentClass<ExtendedWidgetProps>
  };

  const Customwidgets = {
    ...widgets,
    custom: {
      ...widgets.custom,
      Units: UnitWidget as React.FunctionComponent<ExtendedWidgetProps>,
    } as CustomWidgetBinding,
  };

  function parseJson(json: JsonType): JsonType {
    // eslint-disable-next-line no-restricted-syntax
    for (const key in json) {
      if (typeof json[key] === 'object' && json[key] !== null) {
        // If the object already has a 'widget' property, skip it
        if (Object.prototype.hasOwnProperty.call(json[key], 'widget')) {
          // eslint-disable-next-line no-continue
          continue;
        } else if (Object.prototype.hasOwnProperty.call(json[key], 'enum')) {
          json[key].widget = 'Select';
        } else if (json[key].type instanceof Array && ['integer', 'number', 'string'].includes(json[key].type[0])) {
          json[key].widget = 'Text';
        } else if (json[key].type instanceof Array && json[key].type[0] === 'array') {
          json[key].widget = 'GenericList';
        }
        // Recursively parse nested objects
        parseJson(json[key]);
      }
    }
    return json;
  }

  const [schema, setSchema] = React.useState<Immutable.OrderedMap<string | number, any>>(() => createOrderedMap(parseJson(inputSchema)));
  const [store, setStore] = React.useState(() => createStore(values));

  const onInternalChange = React.useCallback((actions: any) => {
    setStore(storeUpdater(actions));
  }, [setStore]);

  return (
    <UIMetaProvider
      widgets={Customwidgets}
      t={relTranslator}
    >
      <UIStoreProvider
        store={store}
        onChange={onInternalChange}
        showValidity
      >
        <GridStack isRoot schema={schema} />
      </UIStoreProvider>

      <Button
        style={{ marginTop: 24 }}
        onClick={() => {
          onSubmit(store.toJS().values);
        }}
      >
        Send!
      </Button>
    </UIMetaProvider>
  );
}
