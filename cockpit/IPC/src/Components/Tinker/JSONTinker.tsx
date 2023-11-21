/* eslint-disable react/function-component-definition */
import React, { useState } from 'react';
import {
  AlertVariant, Button, ClipboardCopy, ClipboardCopyVariant,
} from '@patternfly/react-core';
import { CheckIcon } from '@patternfly/react-icons';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import { useAlertContext } from '../Alert/AlertContext';

interface StringTinkerIface {
  interfaceData: any;
  isChecked: boolean;
}

// eslint-disable-next-line react/function-component-definition
const StringTinker: React.FC<StringTinkerIface> = ({ interfaceData, isChecked }) => {
  const [inputValue, setInputValue] = useState<string>(interfaceData.data);
  const { addAlert } = useAlertContext();
  const slotPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Slots`;
  const signalPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Signals`;

  const handleInputChange = (value: string | number | undefined) => {
    setInputValue(value?.toString() ?? '');
  };

  async function updateDBUS() {
    const client = window.cockpit.dbus(interfaceData.process, { bus: 'system', superuser: 'try' });
    const proxy = client.proxy(interfaceData.interfaceName, interfaceData.direction === 'slot' ? slotPath : signalPath);
    await proxy.wait().then(() => {
      proxy.Tinker(inputValue);
    }).catch((e: any) => {
      addAlert(`Error setting value of ${interfaceData.interfaceName}`, AlertVariant.danger);
      console.log(e);
    });
    addAlert(`Value of ${interfaceData.interfaceName} has been set to ${inputValue}`, AlertVariant.success);
  }

  if (!isChecked) {
    return (
      <p>Unknown</p>);
  }
  return (
    <>
      <ClipboardCopy
        isCode
        hoverTip="Copy"
        isReadOnly
        value={isChecked ? inputValue : ''}
        clickTip="Copied"
        variant={ClipboardCopyVariant.expansion}
        onChange={(_, val) => handleInputChange(val)}
        onKeyDown={(e) => { if (e.key === 'Enter') { updateDBUS(); } }}
        style={{ alignSelf: 'baseline', marginTop: '-.25rem', zIndex: 1 }}
        key={`${interfaceData.interfaceName}-${interfaceData.process}`}
      >
        {JSON.stringify(JSON.parse(interfaceData.Value), null, 2)}
      </ClipboardCopy>

      <Button
        variant="plain"
        onClick={() => updateDBUS()}
        icon={<CheckIcon />}
      />
    </>
  );
};

export default StringTinker;
