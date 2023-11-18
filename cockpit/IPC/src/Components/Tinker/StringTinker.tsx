/* eslint-disable react/function-component-definition */
import React, { useState } from 'react';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import { AlertVariant, Button, TextInput } from '@patternfly/react-core';
import { CheckIcon } from '@patternfly/react-icons';
import { useAlertContext } from '../Alert/AlertContext';

interface StringTinkerIface {
  interfaceData: any;
  isChecked: boolean;
}

const StringTinker: React.FC<StringTinkerIface> = ({ interfaceData, isChecked }) => {
  const [inputValue, setInputValue] = useState<string>(interfaceData.data);
  const { addAlert } = useAlertContext();
  const slotPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Slots`;
  const signalPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Signals`;

  const handleInputChange = (value: string) => {
    setInputValue(value);
  };

  async function updateDBUS() {
    const client = window.cockpit.dbus(interfaceData.process, { bus: 'system', superuser: 'try' });
    const proxy = client.proxy(interfaceData.interfaceName, interfaceData.direction === 'slot' ? slotPath : signalPath);
    await proxy.wait().then(() => {
      addAlert(`Value of ${interfaceData.interfaceName} has been set to ${inputValue}`, AlertVariant.success);
      proxy.Tinker(inputValue);
    }).catch((e: any) => {
      addAlert(`Error setting value of ${interfaceData.interfaceName}`, AlertVariant.danger);
      console.log(e);
    });
  }

  return (
    <>
      <TextInput
        value={isChecked ? inputValue : ''}
        onChange={(_, val) => handleInputChange(val)}
        onKeyDown={(e) => { if (e.key === 'Enter') { updateDBUS(); } }}
        type="text"
        aria-label="tinker text input"
        key={`${interfaceData.interfaceName}-${interfaceData.process}`}
        style={{ minWidth: interfaceData.data === inputValue ? '4rem' : '3rem' }}
        isDisabled={!isChecked}
      />
      {interfaceData.data === inputValue ? null : (
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
