/* eslint-disable react/function-component-definition */
import React, { useState } from 'react';
import { AlertVariant, Button, TextInput } from '@patternfly/react-core';
import { CheckIcon } from '@patternfly/react-icons';
import { useAlertContext } from '../Alert/AlertContext';

interface StringTinkerIface {
  data: any;
  isChecked: boolean;
}

const StringTinker: React.FC<StringTinkerIface> = ({ data, isChecked }) => {
  const [inputValue, setInputValue] = useState<string>(data.proxy.data.Value);
  const { addAlert } = useAlertContext();

  const handleInputChange = (value: string) => {
    setInputValue(value);
  };

  function updateDBUS() {
    data.proxy.Tinker(inputValue);
    addAlert(`Value of ${data.interfaceName} has been set to ${inputValue}`, AlertVariant.success);
  }

  return (
    <>
      <TextInput
        value={inputValue}
        onChange={(_, val) => handleInputChange(val)}
        onKeyDown={(e) => { if (e.key === 'Enter') { updateDBUS(); } }}
        type="text"
        aria-label="tinker text input"
        key={`${data.iface}-${data.process}`}
        style={{ minWidth: data.proxy.data.Value === inputValue ? '4rem' : '3rem' }}
        isDisabled={!isChecked}
      />
      {data.proxy.data.Value === inputValue ? null : (
        <Button
          variant="plain"
          onClick={() => updateDBUS()}
          icon={<CheckIcon />}
        />
      )}
    </>
  );
};

export default StringTinker;
