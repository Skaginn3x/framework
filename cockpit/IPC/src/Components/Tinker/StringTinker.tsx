import React, { useState } from 'react';
import { TextInput, ValidatedOptions } from '@patternfly/react-core';

interface StringTinkerIface {
  data: any;
}

// eslint-disable-next-line react/function-component-definition
const StringTinker: React.FC<StringTinkerIface> = ({ data }) => {
  const [inputValue, setInputValue] = useState(data.Value);

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
      onChange={(e, val) => handleInputChange(val)}
      onKeyDown={handleKeyDown}
      validated={ValidatedOptions.error}
      type="text"
      aria-label="tinker text input"
    />
  );
};

export default StringTinker;
