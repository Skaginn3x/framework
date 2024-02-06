/* eslint-disable react/function-component-definition */
import React, { useCallback, useState } from 'react';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import { AlertVariant, TextInput } from '@patternfly/react-core';
import { useAlertContext } from '../Alert/AlertContext';

interface NumberTinkerIface {
  interfaceData: any;
  isChecked: boolean;
}

const NumberTinker: React.FC<NumberTinkerIface> = ({ interfaceData, isChecked }) => {
  const [error, setError] = useState<string | undefined>(undefined);
  const { addAlert } = useAlertContext();
  const { typeJson } = interfaceData;

  const slotPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Slots`;
  const signalPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Signals`;

  const handleInputChange = (value: string) => {
    console.log(typeJson);
    if (!typeJson) {
      return { value: undefined, error: 'Unknown type' };
    }
    if (typeJson.type[0] !== 'integer' && typeJson.type[0] !== 'number') {
      return { value: undefined, error: 'Unknown type' };
    }

    const val = typeJson.type[0] === 'integer' ? parseInt(value, 10) : parseFloat(value);

    if (typeJson.minimum && val < typeJson.minimum) {
      return { value: val, error: 'Value is too low' };
    }
    if (typeJson.maximum && val > typeJson.maximum) {
      return { value: val, error: 'Value is too high' };
    }

    return { value: val, error: undefined };
  };

  const [inputValue, setInputValue] = useState<number | undefined>(() => handleInputChange(interfaceData.data).value);

  const handleKeyDown = async (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      if (error) {
        addAlert(`Data Validation Error: ${error}`, AlertVariant.danger);
        return;
      }
      if (inputValue === undefined || inputValue === null) {
        addAlert('Input Error: Invalid Input', AlertVariant.danger);
        return;
      }
      const client = window.cockpit.dbus(interfaceData.process, { bus: 'system', superuser: 'try' });
      const proxy = client.proxy(interfaceData.interfaceName, interfaceData.direction === 'slot' ? slotPath : signalPath);
      await proxy.wait().then(() => {
        proxy.Tinker(inputValue);
      });
    }
  };

  const onChangeHandler = useCallback((_: any, val: string) => {
    const result = handleInputChange(val);
    setInputValue(result.value);
    setError(result.error);
  }, []);

  return (
    <TextInput
      value={isChecked ? inputValue : ''}
      onChange={onChangeHandler}
      onKeyDown={handleKeyDown}
      validated={error ? 'error' : 'default'}
      type="number"
      isDisabled={!isChecked}
      aria-label="tinker text input"
      className="tinker-number-input"
      key={`${interfaceData.iface}-${interfaceData.process}`}
      style={{ minWidth: '4rem' }}
    />
  );
};

export default NumberTinker;
