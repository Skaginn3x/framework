/* eslint-disable react/function-component-definition */
import React from 'react';
import * as reactCore from '@patternfly/react-core';

interface TextboxAttributeProps {
  selectedItems: string[];
  setActiveItems: React.Dispatch<React.SetStateAction<string[]>>;
  attributeName: string;
  activeAttributeMenu: string;
}

const TextboxAttribute: React.FC<TextboxAttributeProps> = ({
  selectedItems,
  setActiveItems,
  attributeName,
  activeAttributeMenu,
}) => {
  const [inputValue, setInputValue] = React.useState<string>('');

  const handleInputChange = (val: string) => {
    setInputValue(val);
  };
  console.log(selectedItems);

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter' && inputValue.trim() !== '') {
      // Add the inputValue to the selectedItems
      setActiveItems((prevItems) => [...prevItems, inputValue.trim()]);
      setInputValue(''); // clear the input
    }
  };

  return (
    activeAttributeMenu === attributeName ? (
      <div>
        <reactCore.TextInput
          value={inputValue}
          onChange={(_, val) => handleInputChange(val)}
          onKeyDown={handleKeyDown}
          placeholder={`Add ${attributeName}...`}
        />
      </div>
    )
      : null
  );
};

export default TextboxAttribute;
