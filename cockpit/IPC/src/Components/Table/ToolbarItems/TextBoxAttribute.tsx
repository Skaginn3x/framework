/* eslint-disable react/function-component-definition */
import React from 'react';
import { TextInput } from '@patternfly/react-core';

interface TextboxAttributeProps {
  selectedItems: string[];
  setActiveItems: React.Dispatch<React.SetStateAction<string[]>>;
  attributeName: string;
  activeAttributeMenu: string;
  innerRef: React.RefObject<HTMLInputElement> | null;
}

const TextboxAttribute: React.FC<TextboxAttributeProps> = ({
  selectedItems,
  setActiveItems,
  attributeName,
  activeAttributeMenu,
  innerRef,
}) => {
  const [inputValue, setInputValue] = React.useState<string>('');

  const handleInputChange = (val: string) => {
    setInputValue(val);
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter' && inputValue.trim() !== '') {
      // Add the inputValue to the selectedItems
      if (!selectedItems.includes(inputValue.trim())) {
        setActiveItems((prevItems) => [...prevItems, inputValue.trim()]);
      }
      setInputValue(''); // clear the input
    }
  };

  return (
    activeAttributeMenu === attributeName ? (
      <div>
        <TextInput
          value={inputValue}
          aria-label="textbox-attribute"
          onChange={(_, val) => handleInputChange(val)}
          onKeyDown={handleKeyDown}
          ref={innerRef}
          placeholder={`Search ${attributeName}...`}
        />
      </div>
    )
      : null
  );
};

export default TextboxAttribute;
