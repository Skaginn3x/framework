/* eslint-disable no-restricted-syntax */
/* eslint-disable no-underscore-dangle */
/* eslint-disable no-continue */
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
import { injectPluginStack, validators } from '@ui-schema/ui-schema';
import { GridContainer } from '@ui-schema/ds-material/GridContainer';
import Immutable from 'immutable';
import { AlertVariant, Button } from '@patternfly/react-core';
import { WidgetProps as BaseWidgetProps } from '@ui-schema/ui-schema/Widget';
import { UnitWidget } from './UnitWidget';
import { BooleanWidget } from './BoolWidget';
import { VariantWidget } from './VariantWidget';
import { useAlertContext } from '../Alert/AlertContext';
import { StringWidget } from './StringWidget';
import { ArrayWidget } from './ArrayWidget';

const GridStack = injectPluginStack(GridContainer);

export default function FormGenerator(
  {
    inputSchema, onSubmit, values, intKey,
  }: { inputSchema: any, onSubmit: (data: any) => void, values: any, intKey: any },
) {
  type JsonType = {
    [key: string]: any;
  };

  const { addAlert } = useAlertContext();

  interface ExtendedWidgetProps extends BaseWidgetProps<MuiWidgetBinding>, WithOnChange, WithValue { }

  type CustomWidgetBinding = {
    [type: string]: React.FunctionComponent<ExtendedWidgetProps> | React.ComponentClass<ExtendedWidgetProps>
  };

  const Customwidgets = {
    ...widgets,
    custom: {
      ...widgets.custom,
      Units: UnitWidget as React.FunctionComponent<ExtendedWidgetProps>,
      Variant: VariantWidget as React.FunctionComponent<ExtendedWidgetProps>,
      Boolean: BooleanWidget as React.FunctionComponent<ExtendedWidgetProps>,
      String: StringWidget as React.FunctionComponent<ExtendedWidgetProps>,
      Array: ArrayWidget as React.FunctionComponent<ExtendedWidgetProps>,
    } as CustomWidgetBinding,
    pluginSimpleStack: validators,
  };

  /**
   * Assigns a widget to each type of data in the schema
   * @param key The key of the object
   * @param json The schema
   */
  function assignWidget(key: string, json: JsonType) {
    const type = Array.isArray(json[key].type) ? json[key].type : [json[key].type];

    const illegalType = [
      'number',
      'string',
      'boolean',
      'object',
      'array',
      'null',
    ];
    // count matches between illegalType and type
    // if matches === illegalType.length, then type is illegal
    if (!Object.keys(json[key]).includes('oneOf')) {
      let counter = 0;
      type.forEach((t: string) => {
        if (illegalType.includes(t)) {
          counter += 1;
        }
        if (counter === illegalType.length) {
          addAlert('Illegal type in schema', AlertVariant.danger);
        }
      });
    }

    if ('enum' in json[key]) {
      json[key].widget = 'Select';
    } else if ('oneOf' in json[key]) {
      json[key].widget = 'Variant';
    } else if (type.includes('integer') || type.includes('number')) {
      json[key].widget = 'Units';
    } else if (type.includes('string')) {
      json[key].widget = 'String';
    } else if (type.includes('boolean')) {
      json[key].widget = 'Boolean';
    } else if (type.includes('array')) {
      json[key].widget = 'Array';
      json[key].notSortable = true;
    }
  }

  /**
   *  Parses JSON to select the appropriate widget.
   *  Makes use of custom widgets Units and Variant
   * @param json Schema
   * @returns Parsed Schema with widgets added
   */
  function parseJson(json: JsonType): JsonType {
  // Unwrap single-item 'type' arrays for the root
    if (Array.isArray(json.type) && json.type.length === 1) {
      // eslint-disable-next-line prefer-destructuring
      json.type = json.type[0];
    }

    // eslint-disable-next-line guard-for-in, no-restricted-syntax
    for (const key in json) {
      const entry = json[key];
      if (typeof entry === 'object' && entry !== null) {
      // Skip if the object already has a 'widget' property
        if ('widget' in entry) continue;

        // Unwrap single-item 'type' arrays
        if (Array.isArray(entry.type) && entry.type.length === 1) {
          // eslint-disable-next-line prefer-destructuring
          entry.type = entry.type[0];
        }

        assignWidget(key, json);

        // Recursively parse nested objects
        parseJson(entry);
      }
    }

    return json;
  }

  /**
   * Checks the validity of the data
   * @param data The JsonSchema validity object
   * @param actualData The actual data
   * @returns Boolean indicating validity
   */
  function checkValidity(obj: any, actualData: any) {
    // Check if the current level is valid

    if (actualData && Object.keys(actualData).includes('internal_null_value_do_not_use')) {
      return true;
    }

    if (obj.__valid !== undefined && !obj.__valid && !Array.isArray(actualData)) {
      return false;
    }

    // Recursively check each property if it's an object
    for (const key in obj) {
      if (Object.hasOwn(obj, key) && typeof obj[key] === 'object' && !Array.isArray(obj[key])) {
        const isValid = checkValidity(obj[key], actualData[key] ?? {});
        if (!isValid) {
          return false;
        }
      }
    }

    // If all checks passed, return true
    return true;
  }

  const [schema] = React.useState<Immutable.OrderedMap<string | number, any>>(() => createOrderedMap(parseJson(inputSchema)));
  const [store, setStore] = React.useState(createStore(createOrderedMap(values)));
  const onInternalChange = React.useCallback((actions: any) => {
    setStore(storeUpdater(actions));
  }, [setStore]);

  return (
    <UIMetaProvider
      widgets={Customwidgets}
      t={relTranslator}
      key={`${intKey}-meta-provider`}
    >
      <UIStoreProvider
        store={store}
        onChange={onInternalChange}
        showValidity={false}
        key={`${intKey}-store-provider`}
      >
        <GridStack isRoot schema={schema} showValidity={false} key={`${intKey}-grid`} />
      </UIStoreProvider>

      <Button
        style={{ marginTop: 24 }}
        onClick={() => {
          if (checkValidity(store.toJS().validity, store.toJS().values)) {
            onSubmit(store.toJS());
          } else {
            console.log(store.toJS());
            addAlert('Could not validate configuration', AlertVariant.danger);
          }
        }}
      >
        Send!
      </Button>
    </UIMetaProvider>
  );
}
