/* eslint-disable react/function-component-definition */
import React, { useCallback, useMemo, useState } from 'react';
import { AlertVariant, TextInput } from '@patternfly/react-core';
import { useAlertContext } from '../Alert/AlertContext';

interface NumberTinkerIface {
  data: any;
  isChecked: boolean;
}

const NumberTinker: React.FC<NumberTinkerIface> = ({ data: InterfaceOBJ, isChecked }) => {
  const [error, setError] = useState<string | undefined>(undefined);
  const { addAlert } = useAlertContext();
  const typeJson = useMemo(() => JSON.parse(InterfaceOBJ.proxy.data.Type), [InterfaceOBJ.proxy.data.Type]);
  const type = typeJson.type[0];
  const min = typeJson.minimum as number;
  const max = typeJson.maximum as number;

  const handleInputChange = (value: string) => {
    if (type !== 'integer' && type !== 'number') {
      return { value: undefined, error: 'Unknown type' };
    }

    const val = type === 'integer' ? parseInt(value, 10) : parseFloat(value);

    if (val < min) {
      return { value: val, error: 'Value is too low' };
    }
    if (val > max) {
      return { value: val, error: 'Value is too high' };
    }

    return { value: val, error: undefined };
  };

  const [inputValue, setInputValue] = useState<number | undefined>(() => handleInputChange(InterfaceOBJ.proxy.data.Value).value);

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      if (error) {
        addAlert(`Data Validation Error: ${error}`, AlertVariant.danger);
        return;
      }
      if (inputValue === undefined || inputValue === null) {
        addAlert('Input Error: Invalid Input', AlertVariant.danger);
        return;
      }
      InterfaceOBJ.proxy.Tinker(inputValue);
    }
  };

  const onChangeHandler = useCallback((_: any, val: string) => {
    const result = handleInputChange(val);
    setInputValue(result.value);
    setError(result.error);
  }, []);

  return (
    <TextInput
      value={inputValue}
      onChange={onChangeHandler}
      onKeyDown={handleKeyDown}
      validated={error ? 'error' : 'default'}
      type="number"
      isDisabled={!isChecked}
      aria-label="tinker text input"
      className="tinker-number-input"
      key={`${InterfaceOBJ.iface}-${InterfaceOBJ.process}`}
      style={{ minWidth: '4rem' }}
    />
  );
};

export default NumberTinker;
