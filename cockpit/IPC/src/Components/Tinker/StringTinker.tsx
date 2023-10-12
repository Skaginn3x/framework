/* eslint-disable react/function-component-definition */
import React, { useState } from 'react';
import { TextInput } from '@patternfly/react-core';

interface StringTinkerIface {
  data: any;
}

const StringTinker: React.FC<StringTinkerIface> = ({ data }) => {
  console.log('data:', data);
  const [inputValue, setInputValue] = useState<string>(data.proxy.data.Value);

  const handleInputChange = (value: string) => {
    setInputValue(value);
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      console.log('New Value:', inputValue);
      data.proxy.Tinker(inputValue);
    }
  };

  return (
    <TextInput
      value={inputValue}
      onChange={(_, val) => handleInputChange(val)}
      onKeyDown={handleKeyDown}
      type="text"
      aria-label="tinker text input"
      key={`${data.iface}-${data.process}`}
    />
  );
};

export default StringTinker;
