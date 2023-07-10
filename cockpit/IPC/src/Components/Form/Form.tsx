/* eslint-disable no-param-reassign */
/* eslint-disable @typescript-eslint/no-unused-vars */
import React from 'react';

// "global" ui-config
import { UIMetaProvider, useUIMeta } from '@ui-schema/ui-schema/UIMeta';
// for data-stores / data-binding
import { UIStoreProvider, createEmptyStore, createStore } from '@ui-schema/ui-schema/UIStore';
import { storeUpdater } from '@ui-schema/ui-schema/storeUpdater';

// for validity checking
import { isInvalid } from '@ui-schema/ui-schema/ValidityReporter';
// for deep immutables
import { createOrderedMap } from '@ui-schema/ui-schema/Utils/createMap';
// for `t` keyword support / basic in-schema translation
import { relTranslator } from '@ui-schema/ui-schema/Translate/relT';

// import the widgets for your design-system.
import { widgets } from '@ui-schema/ds-material';
import { injectPluginStack } from '@ui-schema/ui-schema';
import { GridContainer } from '@ui-schema/ds-material/GridContainer';

const GridStack = injectPluginStack(GridContainer);

// export default function Generator(inputSchema: any) {
export default function Generator(
  { inputSchema: actualSchema }: any,
  values: any,
) {
  type JsonType = {
    [key: string]: any;
  };

  function parseJson(json: JsonType): JsonType {
    // eslint-disable-next-line no-restricted-syntax
    for (const key in json) {
      if (typeof json[key] === 'object' && json[key] !== null) {
        // If the object has 'enum', set the widget to 'Select'
        if (Object.prototype.hasOwnProperty.call(json[key], 'enum')) {
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

  const schema = createOrderedMap(parseJson(actualSchema)) as Immutable.OrderedMap<string | number, any>;
  console.log('schema: ', schema);
  const [store, setStore] = React.useState(() => createStore(createOrderedMap(values)));

  const onInternalChange = React.useCallback((actions: any) => {
    setStore(storeUpdater(actions));
  }, [setStore]);

  return (
    <UIMetaProvider
      widgets={widgets}
      t={relTranslator}
    >
      <UIStoreProvider
        store={store}
        onChange={onInternalChange}
        showValidity
      >
        <GridStack isRoot schema={schema} />
      </UIStoreProvider>
    </UIMetaProvider>
  );
}
