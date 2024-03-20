/* eslint-disable react/function-component-definition */
import React from 'react';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import { AlertVariant, Switch } from '@patternfly/react-core';
import { useAlertContext } from '../Alert/AlertContext';

interface BoolTinkerIface {
  interfaceData: any;
  isChecked: boolean;
}

const BoolTinker: React.FC<BoolTinkerIface> = ({ interfaceData, isChecked }) => {
  const { addAlert } = useAlertContext();
  const [value, setValue] = React.useState(interfaceData.data);
  const slotPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Slots`;
  const signalPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Signals`;

  const handleInputChange = async (newValue: boolean) => {
    const client = window.cockpit.dbus(interfaceData.process, { bus: 'system', superuser: 'try' });
    const proxy = client.proxy(interfaceData.interfaceName, interfaceData.direction === 'slot' ? slotPath : signalPath);
    await proxy.wait().then(() => {
      proxy.Tinker(newValue ? 1 : 0).then(() => {
        addAlert(`Value of ${interfaceData.interfaceName} has been set ${newValue ? 'true' : 'false'}`, AlertVariant.success);
        setValue(newValue);
      }).catch((e: any) => {
        addAlert(`Error setting value of ${interfaceData.interfaceName}`, AlertVariant.danger);
        console.log(e);
      });
    });
  };

  return (
    <Switch
      aria-label={`tinker-{${interfaceData.interfaceName}}-{${interfaceData.process}}}`}
      isChecked={isChecked ? value : false}
      onChange={(e, val) => handleInputChange(val)}
      isDisabled={!isChecked}
      key={`${interfaceData.interfaceName}-${interfaceData.process}`}
    />
  );
};

export default BoolTinker;
