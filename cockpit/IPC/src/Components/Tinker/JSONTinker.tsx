/* eslint-disable react/function-component-definition */
import React, { useState } from 'react';
import {
  AlertVariant, Button, ClipboardCopy, ClipboardCopyVariant,
} from '@patternfly/react-core';
import { CheckIcon } from '@patternfly/react-icons';
import { useAlertContext } from '../Alert/AlertContext';

interface StringTinkerIface {
  data: any;
}

const StringTinker: React.FC<StringTinkerIface> = ({ data }) => {
  const [inputValue, setInputValue] = useState<string>(data.proxy.data.Value);
  const { addAlert } = useAlertContext();
  const handleInputChange = (value: string | number | undefined) => {
    setInputValue(value?.toString() ?? '');
  };

  function updateDBUS() {
    data.proxy.Tinker(inputValue);
    addAlert(`Value of ${data.interfaceName} has been set to ${inputValue}`, AlertVariant.success);
  }

  return (
    <>
      <ClipboardCopy
        isCode
        hoverTip="Copy"
        isReadOnly
        value={inputValue}
        clickTip="Copied"
        variant={ClipboardCopyVariant.expansion}
        onChange={(_, val) => handleInputChange(val)}
        onKeyDown={(e) => { if (e.key === 'Enter') { updateDBUS(); } }}
        style={{ alignSelf: 'baseline', marginTop: '-.25rem', zIndex: 1 }}
        key={`${data.iface}-${data.process}`}
      >
        {JSON.stringify(JSON.parse(data.Value), null, 2)}
      </ClipboardCopy>

      <Button
        variant="plain"
        onClick={() => data.proxy.Tinker(inputValue)}
        icon={<CheckIcon />}
      />
    </>
  );
};

export default StringTinker;
