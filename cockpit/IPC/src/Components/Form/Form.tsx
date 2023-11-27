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

const GridStack = injectPluginStack(GridContainer);

export default function FormGenerator(
  { inputSchema, onSubmit, values }: { inputSchema: any, onSubmit: (data: any) => void, values: any },
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

    if ('const' in json[key] && json[key].const !== undefined) {
      json[key].widget = 'Text';
    }

    if ('enum' in json[key]) {
      json[key].widget = 'Select';
    } else if ('oneOf' in json[key]) {
      json[key].widget = 'Variant';
    } else if (type.includes('integer') || type.includes('number')) {
      json[key].widget = 'Units';
    } else if (type.includes('string')) {
      json[key].widget = 'Text';
    } else if (type.includes('boolean')) {
      json[key].widget = 'Boolean';
    } else if (type.includes('array')) {
      json[key].widget = 'GenericList';
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

    if (json.type === 'array') {
      json.widget = 'GenericList';
      json.notSortable = true;
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
  function checkValidity(data: any, actualData: any) {
    if (typeof data !== 'object' || data === null) {
      return true;
    }

    if (Array.isArray(actualData)) {
      return true; // Ignore validity check for arrays
    }

    if (Object.keys(actualData).includes('internal_null_value_do_not_use')) {
      return true;
    }

    if (Object.keys(data).includes('__valid') && data.__valid === false) {
      return false;
    }
    // eslint-disable-next-line guard-for-in
    for (const key in data) {
      if (!Object.keys(actualData).includes(key)) {
        return data[key].__valid ? data[key].__valid : true;
      }
      if (!checkValidity(data[key], actualData[key])) {
        return false;
      }
    }

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
    >
      <UIStoreProvider
        store={store}
        onChange={onInternalChange}
        showValidity={false}
      >
        <GridStack isRoot schema={schema} showValidity={false} />
      </UIStoreProvider>

      <Button
        style={{ marginTop: 24 }}
        onClick={() => {
          console.log(store.toJS());
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
